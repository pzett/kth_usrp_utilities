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
#include <uhd/types/dict.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>
#include <cstdio>
#include <fstream> 

namespace po = boost::program_options;

int UHD_SAFE_MAIN(int argc, char *argv[]){



    if (uhd::set_thread_priority_safe(1,true)) {
       std::cout << "set priority went well " << std::endl;
    };


    //variables to be set by po
    std::string args;
    double seconds_in_future;
    size_t total_num_samps;
    double rate, freq;
    float gain;
    std::string filename_rx, filename_tx;
    uhd::tx_streamer::sptr tx_stream;
    uhd::rx_streamer::sptr rx_stream;
    uhd::device_addr_t dev_addr;
    uhd::usrp::multi_usrp::sptr dev;
    uhd::stream_args_t stream_args;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "simple uhd device address args")
        ("secs", po::value<double>(&seconds_in_future)->default_value(3), "number of seconds in the future to transmit")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(1000), "total number of samples to transmit")
        ("rate", po::value<double>(&rate)->default_value(100e6/16), "rate of outgoing and ingoing samples")
        ("freq", po::value<double>(&freq)->default_value(0), "rf center frequency in Hz")
        ("filename_tx",po::value<std::string>(&filename_tx)->default_value("data_to_usrp.dat"), "tx filename")
        ("filename_rx",po::value<std::string>(&filename_rx)->default_value("data_from_usrp.dat"), "rx filename")        
        ("gain",po::value<float>(&gain)->default_value(0), "gain of transmitter")        
    ;

    

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("tx %s") % desc << std::endl;
        return ~0;
    }

    /* Create buffer storage of transmit signal*/
    std::complex<int16_t> *buffer;
    buffer = new std::complex<int16_t>[total_num_samps];

    /* Create buffer storage of receive signal*/
    std::complex<int16_t> *buffer_rx;
    buffer_rx = new std::complex<int16_t>[total_num_samps];




    /* Create data to be transmitted */
    /* Read input from disc */
    FILE *fp = 0;
    fp = fopen(filename_tx.c_str(), "rb");    
    if (fp == 0){
	perror(filename_tx.c_str());
	return 1;
    }
    int r=fread(buffer, sizeof(uint32_t),total_num_samps, fp);
    printf("r=%d \n",r);
    fclose(fp);


    //create a usrp device and streamer
    dev_addr["addr0"]="192.168.10.2";
    dev = uhd::usrp::multi_usrp::make(dev_addr);    

    dev->set_clock_source("internal");
    dev->set_time_now(uhd::time_spec_t(0.0), 0);

    // Internal variables 
    uhd::clock_config_t my_clock_config; 




    //dev->set_tx_bandwidth(36e6);



    uhd::tune_result_t tr;
    uhd::tune_request_t trq(freq,0); //std::min(tx_rate,10e6));
    tr=dev->set_rx_freq(trq,0);
    tr=dev->set_tx_freq(trq,0);
    

    uhd::dict<std::string, std::string> tx_info;    
    tx_info=dev->get_usrp_tx_info(0);


    dev->set_tx_gain(gain);
    std::cout << tr.to_pp_string() << "\n";
 

    stream_args.cpu_format="sc16";
    tx_stream=dev->get_tx_stream(stream_args);
    rx_stream=dev->get_rx_stream(stream_args);



    //set properties on the device
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;
    dev->set_tx_rate(rate);
    dev->set_rx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (dev->get_tx_rate()/1e6) << std::endl;
    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;


    uhd::tx_metadata_t md;

    
    dev->set_time_now(uhd::time_spec_t(0.0));
    md.start_of_burst = false;
    md.end_of_burst = false;
    md.has_time_spec = true;
    md.time_spec = uhd::time_spec_t(seconds_in_future);

    uint32_t num_samps_sent=0;
    while (num_samps_sent<total_num_samps) {
      int num_tx_samps = tx_stream->send(&buffer[num_samps_sent], total_num_samps-num_samps_sent, md);
      num_samps_sent+=num_tx_samps;
    };

    uhd::rx_metadata_t md_rx;

    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps = total_num_samps;
    stream_cmd.stream_now = false;
    stream_cmd.time_spec = uhd::time_spec_t(seconds_in_future);
    rx_stream->issue_stream_cmd(stream_cmd);

    uint32_t num_samps_recd=0;
    int num_rx_samps, num_samps_left;
    while (num_samps_recd<total_num_samps) {
       num_samps_left=total_num_samps-num_samps_recd;
       num_rx_samps=rx_stream->recv(&buffer_rx[num_samps_recd],num_samps_left, md_rx, seconds_in_future+1);
       num_samps_recd=num_samps_recd+num_rx_samps;
    };

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    // Save output to disc
    std::ofstream s1(filename_rx.c_str(), std::ios::binary);   
    s1.write((char *) buffer_rx,4*total_num_samps); 
    s1.flush(); 
    s1.close(); 


    return 0;
}
