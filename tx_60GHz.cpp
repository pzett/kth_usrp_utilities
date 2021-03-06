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


#include <uhd/utils/thread_priority.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>
#include <cstdio>
#include "board_60GHz.hpp"

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


namespace po = boost::program_options;

board_60GHz_TX *p_my_60GHz_TX;

void my_handler(int s){
     p_my_60GHz_TX->power_down();
     exit(1); 

}


int UHD_SAFE_MAIN(int argc, char *argv[]){



    if (uhd::set_thread_priority_safe(1,true)) {
       std::cout << "set priority went well " << std::endl;
    };


    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);


    //variables to be set by po
    std::string args;
    double seconds_in_future;
    size_t total_num_samps;
    double tx_rate, freq, rf_freq, LOoffset;
    float gain;
    bool forever, use_8bits;
    bool use_external_10MHz; 
    std::string filename;
    uhd::tx_streamer::sptr tx_stream;
    uhd::device_addr_t dev_addr;
    std::string dev_addr_str;
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
      ("freq", po::value<double>(&freq)->default_value(70e6), "center frequency at input of basic daughterboard")
        ("rf_freq", po::value<double>(&rf_freq)->default_value(60e9), "rf center frequency in Hz of 60GHz RX board")
        ("LOoffset", po::value<double>(&LOoffset)->default_value(0), "Offset between main LO and center frequency")
        ("forever",po::value<bool>(&forever)->default_value(true), "run indefinetly")
        ("10MHz",po::value<bool>(&use_external_10MHz)->default_value(false), 
	     "external 10MHz on 'REF CLOCK' connector (true=1=yes)")
      //("PPS",po::value<bool>(&trigger_with_pps)->default_value(false), 
      //      "trigger reception with 'PPS IN' connector (true=1=yes)")
        ("filename",po::value<std::string>(&filename)->default_value("data_to_usrp.dat"), "input filename")
        ("gain",po::value<float>(&gain)->default_value(13), "gain of transmitter(0-13) ")
        ("8bits",po::value<bool>(&use_8bits)->default_value(false), "Use eight bits/sample to increase bandwidth")
       ("dev_addr",po::value<std::string>(&dev_addr_str)->default_value("192.168.10.2"), 
    "IP address of USRP")

    ;

    
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("tx_60GHz.cpp %s") % desc << std::endl;
        return ~0;
    }

    /* Create buffer storage */
    std::complex<int16_t> *buffer;
    buffer = new std::complex<int16_t>[total_num_samps];


    /* Read input from disc */
    FILE *fp = 0;
    fp = fopen(filename.c_str(), "rb");    
    if (fp == 0){
	perror(filename.c_str());
	return 1;
    }
    int r=fread(buffer, sizeof(uint32_t),total_num_samps, fp);
    printf("r=%d \n",r);
    fclose(fp);


    //create a usrp device and streamer
    dev_addr["addr0"]=dev_addr_str;
    dev = uhd::usrp::multi_usrp::make(dev_addr);    


    // Internal variables 
    uhd::clock_config_t my_clock_config; 

    if (!forever) {
      dev->set_time_source("external");
    };

    if (use_external_10MHz) { 
      dev->set_clock_source("external");
    }
    else {
      dev->set_clock_source("internal");
    };



    uhd::usrp::dboard_iface::sptr db_iface;
    db_iface=dev->get_tx_dboard_iface(0);

    board_60GHz_TX  my_60GHz_TX(db_iface);  //60GHz 
    my_60GHz_TX.set_gain(gain); 
    if (rf_freq!=64e9) {
      my_60GHz_TX.set_freq(rf_freq);    // 60GHz
    };
    p_my_60GHz_TX=& my_60GHz_TX;

    uhd::tune_result_t tr;
    uhd::tune_request_t trq(freq,LOoffset); //std::min(tx_rate,10e6));
    tr=dev->set_tx_freq(trq,0);
    

    //dev->set_tx_gain(gain);
    std::cout << tr.to_pp_string() << "\n";
 

    stream_args.cpu_format="sc16";
    if (use_8bits) {

       double max_value=0.0;
       double new_value;
       for (uint32_t i2=0;i2<total_num_samps;i2++){ 
	  new_value=abs(buffer[i2]);
	  if (new_value>max_value) {
	      max_value=new_value;
	  };
       };

      stream_args.otw_format="sc8";
      std::stringstream temp_ss;
      temp_ss << max_value/32768.0*0.5;
      //stream_args.args["peak"]=temp_ss.str();
    }
    else
      stream_args.otw_format="sc16";

    tx_stream=dev->get_tx_stream(stream_args);


    //set properties on the device
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (tx_rate/1e6) << std::endl;
    dev->set_tx_rate(tx_rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (dev->get_tx_rate()/1e6) << std::endl;
    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;


    

    uhd::tx_metadata_t md;

    if (forever) {

      std::cout << "Stop the transmitter by pressing ctrl-c \n";

      md.start_of_burst = true;
      md.end_of_burst = false;
      md.has_time_spec = false;
      
     
      //send the entire buffer, let the driver handle fragmentation

      /*
      num_tx_samps = dev->send(
      buffer, total_num_samps, md,
      uhd::io_type_t::COMPLEX_INT16,
      uhd::device::SEND_MODE_FULL_BUFF);
      */
      tx_stream->send(buffer,total_num_samps,md,60);
      
      md.start_of_burst = false;

      while (1) {

	/*
	num_tx_samps = dev->send(
        buffer, total_num_samps, md,
        uhd::io_type_t::COMPLEX_INT16,
        uhd::device::SEND_MODE_FULL_BUFF
         );
	*/
        tx_stream->send(buffer,total_num_samps,md,3);

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

    //send the entire buffer, let the driver handle fragmentation

    /*
      num_tx_samps = dev->send(
	buffer, total_num_samps, md,
        //&buff.front(), buff.size(), md,
        uhd::io_type_t::COMPLEX_FLOAT32,
        uhd::device::SEND_MODE_FULL_BUFF);
    */
    
    tx_stream->send(buffer,total_num_samps,md,60);
    

    };

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;


    return 0;
}





