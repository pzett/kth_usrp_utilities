//
// Copyright Per Zetterberg, KTH Royal Institute of Technology
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//



#include <fstream>
#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp/subdev_spec.hpp>
#include <uhd/device.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>
#include <uhd/types/clock_config.hpp>
//#include <gruel/realtime.h>
#include "board_60GHz.hpp"

namespace po = boost::program_options;

int UHD_SAFE_MAIN(int argc, char *argv[]){
    
    
    if (!(uhd::set_thread_priority_safe(1,true))) {
      std::cout << "Problem setting thread priority" << std::endl;
      return 1;
    };


    //variables to be set by po
    //double seconds_in_future=0.01;
    size_t total_num_samps;
    double rx_rate, freq, LOoffset, rf_freq, clock_freq;
    bool use_external_10MHz;
    double scaling_8bits;
    std::string filename;
    float gain;
    std::string dev_addr_str;
    uhd::device_addr_t dev_addr;
    uhd::usrp::multi_usrp::sptr dev;
    uhd::tune_result_t tr;
    uhd::stream_args_t stream_args;
    uhd::rx_streamer::sptr rx_stream;


    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(1000), "total number of samples to receive")
        ("rxrate", po::value<double>(&rx_rate)->default_value(100e6/4), "rate of incoming samples")
        ("rf_freq", po::value<double>(&rf_freq)->default_value(60e9), "rf center frequency in Hz of 60GHz RX board")
        ("freq", po::value<double>(&freq)->default_value(60e9), "center frequency at input of basic daughterboard")
        ("LOoffset", po::value<double>(&LOoffset)->default_value(0), "Offset between main LO and center frequency")
        ("10MHz",po::value<bool>(&use_external_10MHz)->default_value(false), "external 10MHz on 'REF CLOCK' connector (true=1=yes)")
      //  ("PPS",po::value<bool>(&trigger_with_pps)->default_value(false), "trigger reception with 'PPS IN' connector (true=1=yes)")
        ("filename",po::value<std::string>(&filename)->default_value("data_from_usrp.dat"), "output filename") 
        ("gain",po::value<float>(&gain)->default_value(0), "set the receiver gain (0-15)") 
        ("8bits_scaling",po::value<double>(&scaling_8bits)->default_value(0.0), 
    "input scaling (invers) when 8bits is used, set to zero to get 16bits")
       ("dev_addr",po::value<std::string>(&dev_addr_str)->default_value("192.168.10.2"), 
    "IP address of USRP")
("clock_freq", po::value<double>(&clock_freq)->default_value(285.714), "Clock frequency of CLK board")

    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    dev_addr["addr0"]=dev_addr_str;
    dev = uhd::usrp::multi_usrp::make(dev_addr);


    //create a usrp device
    std::cout << std::endl;
    uhd::device::sptr udev = dev->get_device();
    dev->set_rx_rate(rx_rate);
    uhd::tune_request_t trq(freq,LOoffset); 
    tr=dev->set_rx_freq(trq);

    uhd::usrp::dboard_iface::sptr db_iface;
    db_iface=dev->get_tx_dboard_iface(0);
       
    board_60GHz_RX my_60GHz_RX(db_iface,clock_freq);    // 60GHz
    my_60GHz_RX.set_gain(gain);    // 60GHz
    if (rf_freq!=64e9) {
      my_60GHz_RX.set_freq(rf_freq);    // 60GHz
    };


    uhd::clock_config_t my_clock_config; 

    if (use_external_10MHz) {
      dev->set_clock_config(my_clock_config); 
      usleep(1e6); // Wait for the 10MHz to lock
    }; 


    if (scaling_8bits<0) {


      stream_args.cpu_format="sc16";
      stream_args.otw_format="sc16";     
      rx_stream=dev->get_rx_stream(stream_args);
      std::complex<int16_t> *d_buffer_rx;


    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);    

      uint32_t buffer_size=rx_stream->get_max_num_samps();
      stream_cmd.num_samps = buffer_size;
      stream_cmd.stream_now = true;    
      stream_cmd.stream_mode=uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS;

      std::cout << "buffer_size=" << buffer_size << "\n";


      d_buffer_rx = new std::complex<int16_t>[buffer_size];   
      rx_stream->issue_stream_cmd(stream_cmd);


       uhd::rx_metadata_t md;
       size_t num_rx_samps_latest_call=0;             
       while (num_rx_samps_latest_call==0) {
	  num_rx_samps_latest_call= 
	   rx_stream->recv(&d_buffer_rx[0],buffer_size, md, 3.0);
       };


      

       double max_value=0.0;
       double new_value;
       for (uint32_t i2=10;i2<num_rx_samps_latest_call;i2++){ 
	  new_value=abs(d_buffer_rx[i2]);
	  if (new_value>max_value) {
	      max_value=new_value;
	  };
       };
       
       std::cout << "max_value=" << max_value << "\n";
       scaling_8bits=max_value*3.0518e-05*abs(scaling_8bits);
       if (scaling_8bits<0.0039062)
	 scaling_8bits=0.0039062;
       std::cout << "scaling_8bits=" << scaling_8bits << "\n";

    };






    stream_args.cpu_format="sc16";
    if (scaling_8bits==0.0) {
      stream_args.otw_format="sc16";     
    } else {
      stream_args.otw_format="sc8";
      std::stringstream temp_ss;
      temp_ss << scaling_8bits;
      stream_args.args["peak"]=temp_ss.str();
    };

    rx_stream=dev->get_rx_stream(stream_args);


    if (use_external_10MHz) { 
      my_clock_config.ref_source=uhd::clock_config_t::REF_SMA; 
    }; 

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("rx_60GHz %s") % desc << std::endl;
        return ~0;
    }


    size_t buffer_size=1000; // Select buffer size
    short *buff_short, *storage_short;
    storage_short=new short[2*total_num_samps]; // Create storage for the 
    // entire received signal to be saved on disk.
    buff_short=new short[2*buffer_size]; // Create storage for a single 
                                                // buffer


    //    dev->set_time_now(uhd::time_spec_t(0.0));
    //std::cout << boost::format("Setting device timestamp to 0...") << std::endl;

    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);    
    
    stream_cmd.num_samps = total_num_samps; //buffer_size;
    stream_cmd.stream_now = true;
    dev->issue_stream_cmd(stream_cmd);


    size_t num_rx_samps=0;
    size_t num_rx_samps_latest_call;

    uhd::rx_metadata_t md;
    while (num_rx_samps<total_num_samps) {
 

       num_rx_samps_latest_call=0;             
       while (num_rx_samps_latest_call==0) {
	 num_rx_samps_latest_call= 
	   rx_stream->recv(&buff_short[0],buffer_size, md, 3.0);
       };

       stream_cmd.stream_mode=uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
       dev->issue_stream_cmd(stream_cmd);


       /* Process the just received buffer */
       int i1=2*num_rx_samps;
       int i2=0;
       while ((i1<(int) (2*total_num_samps)) && (i2<2*((int) num_rx_samps_latest_call ))) 
       {	  
	  storage_short[i1]=buff_short[i2];
	  i1++; i2++;
       };
	 

       num_rx_samps=num_rx_samps+num_rx_samps_latest_call;
    };




    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl; 

    // Save output to disc
    std::ofstream s1(filename.c_str(), std::ios::binary);   


    s1.write((char *) storage_short,4*total_num_samps); 
    s1.flush(); //PZ
    s1.close(); //PZ



    return 0;
}
