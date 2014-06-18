//
// Copyright 2010 Ettus Research LLC
// Copyright 2010-2014 Modified by Per Zetterberg, KTH.
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
    double rx_rate, freq, LOoffset;
    double scaling_8bits;
    std::string filename;
    float gain;
    uhd::device_addr_t dev_addr;
    uhd::usrp::multi_usrp::sptr dev;
    uhd::tune_result_t tr;
    uhd::stream_args_t stream_args;
    uhd::rx_streamer::sptr rx_stream;


    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("nsamp", po::value<size_t>(&total_num_samps)->default_value(1000), "total number of samples to receive")
        ("rxrate", po::value<double>(&rx_rate)->default_value(100e6/4), "rate of incoming samples")
        ("freq", po::value<double>(&freq)->default_value(70e6), "rf center frequency in Hz")
        ("LOoffset", po::value<double>(&LOoffset)->default_value(0), "Offset between main LO and center frequency")
        ("filename",po::value<std::string>(&filename)->default_value("data_from_usrp.dat"), "output filename") 
        ("gain",po::value<float>(&gain)->default_value(0), "set the receiver gain (0-15)") 
        ("8bits_scaling",po::value<double>(&scaling_8bits)->default_value(0.0), 
    "input scaling (invers) when 8bits is used, set to zero to get 16bits")

    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("UHD RX Timed Samples %s") % desc << std::endl;
        return ~0;
    }

    //create a usrp device and streamer
    dev_addr["addr0"]="192.168.10.2";
    dev_addr["addr1"]="192.168.20.2";
    dev = uhd::usrp::multi_usrp::make(dev_addr);



    #if 1
    uhd::usrp::dboard_iface::sptr db_iface0, db_iface1; 
    db_iface0=dev->get_tx_dboard_iface(0);
    db_iface1=dev->get_tx_dboard_iface(1);

    board_60GHz_RX  my_60GHz_RX0(db_iface0);  //60GHz 
    my_60GHz_RX0.set_gain(gain); // 60GHz
    board_60GHz_RX  my_60GHz_RX1(db_iface1);  //60GHz 
    my_60GHz_RX1.set_gain(gain); // 60GHz


    uhd::tune_request_t trq(freq,LOoffset); 
    tr=dev->set_rx_freq(trq,0);
    tr=dev->set_rx_freq(trq,1);
    #endif


    std::cout << "rx_rate=" << rx_rate << "\n";
    dev->set_rx_rate(rx_rate);


    //make mboard 1 a slave over the MIMO Cable
    dev->set_clock_source("mimo", 1);
    dev->set_time_source("mimo", 1);

    //set time on the master (mboard 0)
    dev->set_time_now(uhd::time_spec_t(0.0), 0);

    //sleep a bit while the slave locks its time to the master
    usleep(100e3);

    //dev->set_rx_subdev_spec(uhd::usrp::subdev_spec_t("A:A"), 0); 


 // Create storage for the entire received signal.




    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    uhd::rx_metadata_t md;
    size_t num_rx_samps_latest_call;


    std::cout << "scaling_8bits=" << scaling_8bits << "\n";
    if (scaling_8bits<-0) {


      std::vector<size_t> channels;
      channels.push_back(0);
      channels.push_back(1);
      stream_args.channels=channels;

      stream_args.cpu_format="sc16";
      stream_args.otw_format="sc16";     
      rx_stream=dev->get_rx_stream(stream_args);
      std::complex<int16_t> *d_buffer_rx;

      uint32_t buffer_size=rx_stream->get_max_num_samps();
      stream_cmd.num_samps = buffer_size;
      stream_cmd.stream_now = false;
      stream_cmd.time_spec = uhd::time_spec_t(0.3);


      std::vector<std::complex<int16_t> *> d_rx_buffers_short;
      uint32_t no_chan=2;


      for (uint32_t i1=0;i1<no_chan;i1++) {
        d_buffer_rx = new std::complex<int16_t>[buffer_size];   
        d_rx_buffers_short.push_back(d_buffer_rx);
      };

      rx_stream->issue_stream_cmd(stream_cmd);

       num_rx_samps_latest_call=0;             
       while (num_rx_samps_latest_call==0) {
	  num_rx_samps_latest_call= 
	   rx_stream->recv(d_rx_buffers_short,buffer_size, md, 3.0);
       };

       double max_value=0.0;
       double new_value;
       for (uint32_t i1=0;i1<no_chan;i1++) {
	 for (uint32_t i2=10;i2<num_rx_samps_latest_call;i2++){ 
	    new_value=abs(d_rx_buffers_short[i1][i2]);
	    if (new_value>max_value) {
	      max_value=new_value;
	    };
         };
       };
       
       std::cout << "max_value=" << max_value << "\n";
       scaling_8bits=max_value*3.0518e-05*abs(scaling_8bits);
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

    std::vector<size_t> channels;
    channels.push_back(0);
    channels.push_back(1);
    stream_args.channels=channels;

    rx_stream=dev->get_rx_stream(stream_args);

   std::complex<int16_t> *d_buffer_rx;
   std::vector<std::complex<int16_t> *> d_rx_buffers;
   uint32_t no_chan=2;
   
   for (uint32_t i1=0;i1<no_chan;i1++) {
     d_buffer_rx = new std::complex<int16_t>[total_num_samps];   
     d_rx_buffers.push_back(d_buffer_rx);
   };


   // Create storage for a single buffer
   uint32_t buffer_size=rx_stream->get_max_num_samps();
   std::vector<std::complex<int16_t> *> d_rx_buffers_short;

   for (uint32_t i1=0;i1<no_chan;i1++) {
     d_buffer_rx = new std::complex<int16_t>[buffer_size];   
     d_rx_buffers_short.push_back(d_buffer_rx);
   };


    
  
    stream_cmd.num_samps = total_num_samps;
    stream_cmd.stream_now = false;
    stream_cmd.time_spec = uhd::time_spec_t(1.3);
    rx_stream->issue_stream_cmd(stream_cmd);


    size_t num_rx_samps=0;


    std::cout << "buffer_size=" << buffer_size  <<  "\n";

    while (num_rx_samps<total_num_samps) {
 

       num_rx_samps_latest_call=0;             
       while (num_rx_samps_latest_call==0) {
	 num_rx_samps_latest_call= 
	   rx_stream->recv(d_rx_buffers_short,buffer_size, md, 3.0);
       };

       /* Process the just received buffer */
       int i1=num_rx_samps;
       int i2=0;

       
       while ((i1<(int) (total_num_samps)) && (i2< ((int) num_rx_samps_latest_call  ))) 
       {
	 (d_rx_buffers[0])[i1]=(d_rx_buffers_short[0])[i2];
	 (d_rx_buffers[1])[i1]=(d_rx_buffers_short[1])[i2];
	 i1++;
	 i2++;
       };
       
	 
       num_rx_samps=num_rx_samps+num_rx_samps_latest_call;
    };
    



    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl; 

    // Save output to disc

    // Save output to disc
    std::ofstream s1(filename.c_str(), std::ios::binary);   // PZ
    for (uint32_t i1=0;i1<no_chan;i1++) {
      d_buffer_rx = (std::complex<short int>*) d_rx_buffers[i1];
      s1.write((char *) d_buffer_rx ,4*total_num_samps); //ZP
    };
    s1.flush(); //PZ
    s1.close(); //PZ


    return 0;
}
