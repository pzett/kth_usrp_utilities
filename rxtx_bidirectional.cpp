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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

namespace po = boost::program_options;

class signal_processing {  // Replace this class to make your own implementation

  public:
  
  signal_processing(int total_num_samps, int num_repeats, int send_to_listener,
     int buffer_size, int socket, std::string filename_rx, 
		    std::string filename_tx);


  /* Process the latest received buffer and generate a transmit buffer. */
  /* Defined below UHD_SAFE_MAIN. */
  int process_buffers(std::complex<int16_t> *process_buffer_rx,
       std::complex<int16_t> *process_buffer_tx); 

  ~signal_processing(void);
   

  private:
  
  // Member variables
  int m_num_samps_transmitted;
  int m_total_num_samps;
  int m_num_repeats_done;
  int m_num_repeats;
  int m_send_to_listener;
  int m_buffer_size;
  int m_socket;
  int m_buffer_ix;
  std::string m_filename_rx, m_filename_tx;  
  

  /* Hold the entire signal to be transmitted */
  std::complex<int16_t> *buffer_tx;

  /* Hold the entire signal received */
  std::complex<int16_t> *buffer_rx;

  /* Buffer to send data to listener */
  int16_t *buffer_to_listener;
  
};


int UHD_SAFE_MAIN(int argc, char *argv[]){



    if (uhd::set_thread_priority_safe(1,true)) {
       std::cout << "set priority went well " << std::endl;
    };


    #if 0
    int portno=30000;
    //int s=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in  server;
    struct hostent *hp;
    struct in_addr ipv4addr;
    

    inet_pton(AF_INET, "127.0.0.1", &ipv4addr);

    bzero((char *)&server, sizeof (server));
    hp = gethostbyaddr(&ipv4addr,sizeof(ipv4addr), AF_INET);
    bcopy(hp->h_addr, (char *)&server.sin_addr,
          hp->h_length);  
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons(portno);
    int s = socket(hp->h_addrtype, SOCK_STREAM, 0);
    if (s < 0) 
      std::cerr << "ERROR opening socket";

    connect(s, (struct sockaddr *)&server, sizeof(server));
    usleep(1003);

    char buffer[6];
    usleep(1e6);
    buffer[0]=65;
    buffer[1]=66;
    buffer[2]=67;
    buffer[3]=68;
    buffer[4]=10;
    buffer[5]=0;

    short nb[5];
    nb[0]=htons(23); nb[1]=htons(-24); nb[2]=htons(77); 
    nb[3]=htons(-18);  nb[4]=htons(-33); 

    std::cout << "strlen=" << strlen(buffer) << "\n";

    
    for (int i1=0;i1<10;i1++) {
      std::cout << "buffer[" << i1 << "]=" << (unsigned int)buffer[i1] << "\n";
    };
    

    //int n = write(s,buffer,strlen(buffer));
    int n = write(s,nb,sizeof(nb));

    std::cout << "n=" << n << "\n";
    if (n < 0) 
      std::cerr << "ERROR writing to socket";

    close(s);
    usleep(100e6);
    #endif


    //variables to be set by po
    std::string args;
    double seconds_in_future;
    size_t total_num_samps, total_num_repeats;
    int send_to_listener;
    
    double rate, freq_tx, freq_rx;
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
        ("secs", po::value<double>(&seconds_in_future)->default_value(0.5), "number of seconds in the future to transmit")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(1000), "Total number of samples to transmit and receive")
        ("nrep", po::value<size_t>(&total_num_repeats)->default_value(1), "Total number of repeats")
        ("rate", po::value<double>(&rate)->default_value(100e6/8), "rate of outgoing and ingoing samples")
        ("freq_rx", po::value<double>(&freq_rx)->default_value(20e6), "receive center frequency in Hz")
        ("freq_tx", po::value<double>(&freq_tx)->default_value(20e6), "transmit center frequency in Hz")
        ("filename_tx",po::value<std::string>(&filename_tx)->default_value("data_to_usrp.dat"), "tx filename")
        ("filename_rx",po::value<std::string>(&filename_rx)->default_value("data_from_usrp.dat"), "rx filename")        
        ("gain",po::value<float>(&gain)->default_value(0), "gain of transmitter")        
        ("n",po::value<int>(&send_to_listener)->default_value(0), "Every n:th received buffer is sent to a client listening on port 3000. ")        

    ;

    

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")){
        std::cout << boost::format("rxtx_bidirectional %s") % desc << std::endl;
        return ~0;
    }





    int s; 

    if (send_to_listener>0) {

      struct sockaddr_in  server;
      struct hostent *hp;
      struct in_addr ipv4addr;
      int portno=30000;    

      inet_pton(AF_INET, "127.0.0.1", &ipv4addr);

      bzero((char *)&server, sizeof (server));
      hp = gethostbyaddr(&ipv4addr,sizeof(ipv4addr), AF_INET);
      bcopy(hp->h_addr, (char *)&server.sin_addr,
          hp->h_length);  
      server.sin_family = hp->h_addrtype;
      server.sin_port = htons(portno);
      s = socket(hp->h_addrtype, SOCK_STREAM, 0);
      if (s < 0) 
	std::cerr << "ERROR opening socket";

      connect(s, (struct sockaddr *)&server, sizeof(server));
    } ;



    int process_buffer_size=9000; // Buffer size in processing
    int tx_ahead_buffers=3; // Number of buffers transmitted before starting
                            // to receive.

    signal_processing sp(total_num_samps,total_num_repeats,send_to_listener,s,
	       process_buffer_size,filename_rx,filename_tx);


    std::complex<int16_t> *process_buffer_tx;
    process_buffer_tx = new std::complex<int16_t>[process_buffer_size];

    std::complex<int16_t> *process_buffer_rx;
    process_buffer_rx = new std::complex<int16_t>[process_buffer_size];

    
    /* Create buffer storage for trailing zeros */
    std::complex<int16_t> *buffer_zeros;
    buffer_zeros = new std::complex<int16_t>[process_buffer_size]();
    

    //create a usrp device and streamer
    dev_addr["addr0"]="192.168.10.2";
    dev = uhd::usrp::multi_usrp::make(dev_addr);    

    dev->set_clock_source("internal");
    dev->set_time_now(uhd::time_spec_t(0.0), 0);

    // Internal variables 
    uhd::clock_config_t my_clock_config; 



    uhd::tune_result_t tr;
    uhd::tune_request_t trq_rx(freq_rx,0); //std::min(tx_rate,10e6));
    tr=dev->set_rx_freq(trq_rx,0);

    uhd::tune_request_t trq_tx(freq_tx,0); //std::min(tx_rate,10e6));
    tr=dev->set_tx_freq(trq_tx,0);

    

    uhd::dict<std::string, std::string> tx_info;    
    tx_info=dev->get_usrp_tx_info(0);


    dev->set_tx_gain(gain);
    std::cout << tr.to_pp_string() << "\n";
 

    stream_args.cpu_format="sc16";
    tx_stream=dev->get_tx_stream(stream_args);
    rx_stream=dev->get_rx_stream(stream_args);



    //set properties on the device
    dev->set_tx_rate(rate);
    dev->set_rx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (dev->get_tx_rate()/1e6) << std::endl;
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (dev->get_rx_rate()/1e6) << std::endl;
    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
    uhd::tx_metadata_t md;
  


    dev->set_time_now(uhd::time_spec_t(0.0));



    uhd::rx_metadata_t md_rx;
    //uhd::stream_cmd_t 
    //  stream_cmd(uhd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);

    uhd::stream_cmd_t 
      stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);


    stream_cmd.num_samps = total_num_samps;
    stream_cmd.stream_now = false;
    stream_cmd.time_spec = uhd::time_spec_t(seconds_in_future);
    rx_stream->issue_stream_cmd(stream_cmd);


    md.start_of_burst = true;
    md.end_of_burst = false;
    md.has_time_spec = true;
    md.time_spec = uhd::time_spec_t(seconds_in_future);

    for (int i1=tx_ahead_buffers;i1>0;i1--) {
      tx_stream->send(buffer_zeros, 
		    process_buffer_size , md);
      md.start_of_burst = false;
      md.has_time_spec = false;
    };
 
    md.start_of_burst = false;
    int return_value=1;

    while (return_value!=0) {

       rx_stream->recv(process_buffer_rx,
	    process_buffer_size, md_rx, seconds_in_future+1);
       return_value=sp.process_buffers(process_buffer_rx, process_buffer_tx);
       tx_stream->send(process_buffer_tx, 
		    process_buffer_size , md); 

      
    };

    md.end_of_burst = true;
    tx_stream->send(buffer_zeros, process_buffer_size , md);


    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;



    return 0;
}



signal_processing::signal_processing(int total_num_samps, int num_repeats,
		      int send_to_listener,int socket,int buffer_size,
			 std::string filename_rx, std::string filename_tx) {
  
  m_total_num_samps=total_num_samps;
  m_num_repeats=num_repeats;
  m_num_samps_transmitted=0;
  m_num_repeats_done=0;
  m_send_to_listener=send_to_listener;
  m_socket=socket;
  m_buffer_size=buffer_size;
  m_buffer_ix=0;

  m_filename_rx=filename_rx;
  m_filename_tx=filename_tx;


  buffer_tx=new std::complex<int16_t>[total_num_samps];
  buffer_rx=new std::complex<int16_t>[total_num_samps];
  buffer_to_listener=new short[2*m_buffer_size];


  /* Create data to be transmitted */
  /* Read input from disc */
  FILE *fp = 0;
  fp = fopen(m_filename_tx.c_str(), "rb");    
  if (fp == 0){
	perror(m_filename_tx.c_str());
        throw ("Failed to open file");
  }
  int r=fread(buffer_tx, sizeof(uint32_t),total_num_samps, fp);
  printf("r=%d \n",r);
  fclose(fp);



};


int signal_processing::process_buffers(
	     std::complex<int16_t> *process_buffer_rx,
	     std::complex<int16_t> *process_buffer_tx){

  bool all_done=false;

  
  if (m_send_to_listener>0) {


    if (((m_num_repeats_done % m_send_to_listener)==0) && (m_buffer_ix==0)) {

      int i2=0;
      for (int i1=0;i1<m_buffer_size;i1++) {
	buffer_to_listener[i2++]=htons(real(process_buffer_rx[i1]));
      };
      for (int i1=0;i1<m_buffer_size;i1++) {
	buffer_to_listener[i2++]=htons(imag(process_buffer_rx[i1]));
      };

      write(m_socket,buffer_to_listener,m_buffer_size*4);


    };
 };
    m_buffer_ix++;

  for (int i1=0;i1<m_buffer_size;i1++) {
    if (m_num_samps_transmitted>=m_total_num_samps) {
         m_num_samps_transmitted=0;
         m_num_repeats_done++;
	 m_buffer_ix=0;
    };

    if (m_num_repeats_done==m_num_repeats) {
      process_buffer_tx[i1]=0.0;
      all_done=true;
    } else {
      process_buffer_tx[i1]=buffer_tx[m_num_samps_transmitted];
      buffer_rx[m_num_samps_transmitted]=
	process_buffer_rx[i1];
      

      m_num_samps_transmitted++;
    };
  };


  
  if (all_done)
    return 0;
  else
    return 1;

};


signal_processing::~signal_processing(void) {
  // Save output to disc
  std::ofstream s1(m_filename_rx.c_str(), std::ios::binary);   
  s1.write((char *) buffer_rx,4*m_total_num_samps); 
  s1.flush(); 
  s1.close(); 
  
  delete buffer_rx;
  delete buffer_tx;
  delete buffer_to_listener;

};

