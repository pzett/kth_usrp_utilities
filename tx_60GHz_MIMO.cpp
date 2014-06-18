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

// This program has the same functionality as tx.cpp but for the // 60GHz tranceiver.

#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>
#include <cstdio>
#include "board_60GHz.hpp"

namespace po = boost::program_options;


int UHD_SAFE_MAIN(int argc, char *argv[]){



    if (uhd::set_thread_priority_safe(1,true)) {
       std::cout << "set priority went well " << std::endl;
    };


    //variables to be set by po
    std::string args;
    double seconds_in_future;
    size_t total_num_samps;
    double tx_rate, freq, LOoffset;
    float gain;
    bool forever, use_8bits;
    std::string filename;
    uhd::tx_streamer::sptr tx_stream;
    uhd::device_addr_t dev_addr;
    uhd::usrp::multi_usrp::sptr dev;
    uhd::stream_args_t stream_args;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "simple uhd device address args")
        ("secs", po::value<double>(&seconds_in_future)->default_value(3), "number of seconds in the future to transmit")
        ("nsamp", po::value<size_t>(&total_num_samps)->default_value(1000), "total number of samples to transmit")
        ("txrate", po::value<double>(&tx_rate)->default_value(100e6/4), "rate of outgoing samples")
        ("freq", po::value<double>(&freq)->default_value(70e6), "rf center frequency in Hz")
        ("LOoffset", po::value<double>(&LOoffset)->default_value(0), "Offset between main LO and center frequency")
        ("forever",po::value<bool>(&forever)->default_value(true), "run indefinetly")
      //("PPS",po::value<bool>(&trigger_with_pps)->default_value(false), 
      //      "trigger reception with 'PPS IN' connector (true=1=yes)")
        ("filename",po::value<std::string>(&filename)->default_value("data_to_usrp.dat"), "input filename")
        ("gain",po::value<float>(&gain)->default_value(13), "gain of transmitter(0-13) ")
        ("8bits",po::value<bool>(&use_8bits)->default_value(false), "Use eight bits/sample to increase bandwidth")

    ;

    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("tx %s") % desc << std::endl;
        return ~0;
    }

   uint32_t no_chan=2;
   // Create storage for the entire transmitted signal.
   std::complex<int16_t> *d_buffer;
   std::vector<const void *> d_tx_buffers; 
   for (uint32_t i1=0;i1<no_chan;i1++) {
     d_buffer = new std::complex<int16_t>[total_num_samps];   
     d_tx_buffers.push_back(d_buffer);
   };


    /* Read input from disc */
    FILE *fp = 0;
    fp = fopen(filename.c_str(), "rb");    
    if (fp == 0){
	perror(filename.c_str());
	return 1;
    }
    for (uint32_t i1=0;i1<no_chan;i1++) {
      d_buffer = (std::complex<short int>*) d_tx_buffers[i1];
      fread(d_buffer, sizeof(uint32_t),total_num_samps, fp);
    };
    fclose(fp);



    //create a usrp device and streamer
    dev_addr["addr0"]="192.168.10.2";
    dev_addr["addr1"]="192.168.20.2";
    dev = uhd::usrp::multi_usrp::make(dev_addr);    


    // Internal variables 
    uhd::clock_config_t my_clock_config; 

    //make mboard 1 a slave over the MIMO Cable
    dev->set_clock_source("mimo", 1);
    dev->set_time_source("mimo", 1);

    //set time on the master (mboard 0)
    dev->set_time_now(uhd::time_spec_t(0.0), 0);

    //sleep a bit while the slave locks its time to the master
    usleep(100e3);



    uhd::usrp::dboard_iface::sptr db_iface0, db_iface1; 
    db_iface0=dev->get_tx_dboard_iface(0);
    db_iface1=dev->get_tx_dboard_iface(1);

    board_60GHz_TX  my_60GHz_TX0(db_iface0);  //60GHz 
    my_60GHz_TX0.set_gain(gain); // 60GHz
    board_60GHz_TX  my_60GHz_TX1(db_iface1);  //60GHz 
    my_60GHz_TX1.set_gain(gain); // 60GHz

    uhd::tune_result_t tr;
    uhd::tune_request_t trq(freq,LOoffset); //std::min(tx_rate,10e6));
    tr=dev->set_tx_freq(trq,0);
    tr=dev->set_tx_freq(trq,1);
   

    //dev->set_tx_gain(gain);
    std::cout << tr.to_pp_string() << "\n";
 

    stream_args.cpu_format="sc16";
    if (use_8bits)
      stream_args.otw_format="sc8";
    else
      stream_args.otw_format="sc16";
    std::vector<size_t> channels;
    channels.push_back(0);
    channels.push_back(1);
    stream_args.channels=channels;

    tx_stream=dev->get_tx_stream(stream_args);


    //set properties on the device
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (tx_rate/1e6) << std::endl;
    dev->set_tx_rate(tx_rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (dev->get_tx_rate()/1e6) << std::endl;

    

    

    uhd::tx_metadata_t md;

    if (forever) {

      std::cout << "Stop the transmitter by pressing ctrl-c \n";

      md.start_of_burst = true;
      md.end_of_burst = false;
      md.has_time_spec = false;
      
      tx_stream->send(d_tx_buffers,total_num_samps,md,60);
      
      md.start_of_burst = false;

      while (1) {

        tx_stream->send(d_tx_buffers,total_num_samps,md,3);

	md.start_of_burst = false;
	md.end_of_burst = false;
	md.has_time_spec = false;

      }
    }
    else
    {
    
    dev->set_time_now(uhd::time_spec_t(0.0));
    md.start_of_burst = true;
    md.end_of_burst = true;
    md.has_time_spec = true;
    md.time_spec = uhd::time_spec_t(seconds_in_future);

    
    tx_stream->send(d_tx_buffers,total_num_samps,md,60);
    

    };

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;


    return 0;
}





