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

namespace po = boost::program_options;


class board_60GHz_base {

  public:  

  board_60GHz_base(uhd::usrp::dboard_iface::sptr db_iface,
		      uhd::usrp::dboard_iface::unit_t unit,
		 uint16_t enable_hmc, uint16_t data_in_hmc,
  uint16_t clk_hmc, uint16_t data_out_hmc, uint16_t reset_hmc, 
  uint16_t chip_addr);

  void write_row(uint16_t row_num, uint32_t value);
  uint16_t read_row(uint16_t row_num);




  private:

  uhd::usrp::dboard_iface::aux_dac_t m_which_dac;
  
  uint16_t m_enable_hmc, m_data_in_hmc, m_data_out_hmc;
  uint16_t m_clk_hmc, m_reset_hmc, m_dummy;
  uint16_t mask_read, mask_write, mask_all;
  uint16_t m_chip_addr;

  uhd::usrp::dboard_iface::sptr m_db_iface;
  uhd::usrp::dboard_iface::unit_t m_unit;
 
};



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
    bool use_external_10MHz; 
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
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(1000), "total number of samples to transmit")
        ("txrate", po::value<double>(&tx_rate)->default_value(100e6/4), "rate of outgoing samples")
        ("freq", po::value<double>(&freq)->default_value(70e6), "rf center frequency in Hz")
        ("LOoffset", po::value<double>(&LOoffset)->default_value(0), "Offset between main LO and center frequency")
        ("forever",po::value<bool>(&forever)->default_value(true), "run indefinetly")
        ("10MHz",po::value<bool>(&use_external_10MHz)->default_value(false), 
	     "external 10MHz on 'REF CLOCK' connector (true=1=yes)")
      //("PPS",po::value<bool>(&trigger_with_pps)->default_value(false), 
      //      "trigger reception with 'PPS IN' connector (true=1=yes)")
        ("filename",po::value<std::string>(&filename)->default_value("data_to_usrp.dat"), "input filename")
        ("gain",po::value<float>(&gain)->default_value(0), "gain of transmitter")
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
    dev_addr["addr0"]="192.168.10.2";
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

    int data_out_hmc=1;
    int data_in_hmc=0;
    int clk_hmc=2;
    int enable_hmc=3;
    int reset_hmc=4;

    //int chip_addr=2+4; // TX
    int chip_addr=1+2+4; // RX

    board_60GHz_base my_60GHz_board(db_iface,uhd::usrp::dboard_iface::UNIT_TX,
		 enable_hmc, data_in_hmc,
				    clk_hmc, data_out_hmc, reset_hmc,chip_addr);


    //my_60GHz_board.write_row(0,0); // Power everything on


    //my_60GHz_board.write_row(1,0); // Highest Q and everything on
    //my_60GHz_board.write_row(2,0xF0); // Normal operation

    #if 0
    usleep(1e6);
    my_60GHz_board.write_row(3,4) ;
    usleep(1e6);    
    std::cout << "Read=" << my_60GHz_board.read_row(3) << "\n"; 
    exit(1);
    #endif

    #if 0 // TX

    my_60GHz_board.write_row(0,0); // Power on everything
    my_60GHz_board.write_row(1,0); // Power on and highest Q of filter.
    my_60GHz_board.write_row(2,240); // Taken from PC app.
    my_60GHz_board.write_row(3,31); // Taken from PC app.
    my_60GHz_board.write_row(4,63); // Normal operation
    my_60GHz_board.write_row(5,244); // Normal operation
    my_60GHz_board.write_row(6,143); // 

    int l_tx_gain=13; // 0:13. Increasing gain.
    my_60GHz_board.write_row(7,15+16*(13-l_tx_gain)); 
                                   // Highest gain + normal operation
    
    my_60GHz_board.write_row(8,191); // normal operation
    my_60GHz_board.write_row(9,111); // normal operation


    // Table 10. 285.7143MHz Reference
    // Frequency(GHz)    DIVRATIO            BAND

    /*      57             00001             000
            57.5           00010             000
            58             00011             001
            58.5           00100             001
            59             00101             010
            59.5           00110             010
            60             00111             011
            60.5           01000             011
            61             01001             100
            61.5           01010             100
            62             01011             101
            62.5           01100             101
            63             01101             110
            63.5           01110             110
            64             01111             111  */


    my_60GHz_board.write_row(10,240); // 240+DIVRATIO<4>
    my_60GHz_board.write_row(11,16*(1+2+4)+2*3+1); // 16*DIVRATIO<3:0>+2*BAND+1


    my_60GHz_board.write_row(12,95); // Syntesizer parameters (lock window)
    my_60GHz_board.write_row(13,128); // normal operation (synths on)
    my_60GHz_board.write_row(14,118); // normal operation


    #endif 


    #if 1 // RX

    my_60GHz_board.write_row(0,128); // Everthing on except ASK mod.
    int bb_gain1=1; // 0-3
    int bb_gain2=1; // 0-3
    int bb_att1=3-bb_gain1;
    int bb_att2=3-bb_gain2;
    my_60GHz_board.write_row(1,bb_att2+4*bb_att1); // Power on + gain control

    int bb_gain_fineI=5; // 0-5
    int bb_gain_fineQ=5; // 0-5

    int bb_att_fineI=5-bb_gain_fineI;
    int bb_att_fineQ=5-bb_gain_fineQ;


    my_60GHz_board.write_row(2,4*bb_att_fineQ+32*bb_att_fineI); 
                                               // Normal operation

    int bb_low_pass_corner=0; // 0=>1.4GHz, 1=>500MHz, 2=> 300MHz, 3=>200MHz.
    int bb_high_pass_corner=0; // 0=>30kHz, 1=>300kHz, 2=>1.5MHz.

    my_60GHz_board.write_row(3,3+16*bb_high_pass_corner+64*bb_low_pass_corner);
                                            // Normal operation
    
    my_60GHz_board.write_row(4,158); // Normal operation

    int if_gain=15; // 0-15
    int if_att=15-if_gain;

    my_60GHz_board.write_row(5,15+16*if_att); // Normal operation
    my_60GHz_board.write_row(6,191); // Normal operation
    my_60GHz_board.write_row(7,109); // Normal operation
    my_60GHz_board.write_row(8,128); // Normal operation
    my_60GHz_board.write_row(9,0); // Normal operation
    my_60GHz_board.write_row(10,240); // 240+DIVRATIO<4>
    my_60GHz_board.write_row(11,16*(1+2+4)+2*3+1); // 16*DIVRATIO<3:0>+2*BAND+1
    my_60GHz_board.write_row(12,95); // Normal operation
    my_60GHz_board.write_row(13,128); // Normal operation
    my_60GHz_board.write_row(14,118); // Normal operation

    #endif 

    for (int i1=0;i1<15;i1++) {
      std::cout << "reg=" << i1 << " value=" << my_60GHz_board.read_row(i1) 
          << "\n";
    };

    
    #if 1
    for (int i1=0;i1<10;i1++) {
      std::cout << "Lock state=" << (my_60GHz_board.read_row(15)>> 6 ) 
		<< " (01=lock)" << "\n";
      usleep(1e6);
    };
    #endif




    //dev->set_tx_bandwidth(36e6);


    std::cout << "freq=" << freq << "\n";
    std::cout << "LOoffset=" << LOoffset << "\n";
    

    uhd::tune_result_t tr;
    uhd::tune_request_t trq(freq,LOoffset); //std::min(tx_rate,10e6));
    tr=dev->set_tx_freq(trq,0);
    

    //dev->set_tx_gain(gain);
    std::cout << tr.to_pp_string() << "\n";
 

    stream_args.cpu_format="sc16";
    if (use_8bits)
      stream_args.otw_format="sc8";
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


board_60GHz_base::board_60GHz_base(uhd::usrp::dboard_iface::sptr db_iface,
		      uhd::usrp::dboard_iface::unit_t unit,
		 uint16_t enable_hmc, uint16_t data_in_hmc,
				   uint16_t clk_hmc, uint16_t data_out_hmc, uint16_t reset_hmc, uint16_t chip_addr )
{


  std::vector<uint16_t> gpio_lines;

  m_dummy=15; // We do dummy reads from this line.

  gpio_lines.push_back(enable_hmc);
  gpio_lines.push_back(data_in_hmc);
  gpio_lines.push_back(clk_hmc);
  gpio_lines.push_back(data_out_hmc);
  gpio_lines.push_back(reset_hmc);

  for (uint32_t i1=0;i1<gpio_lines.size();i1++) {
    if (gpio_lines[i1]==m_dummy) {
      std::cerr << "Use of GPIO line " << m_dummy << "is not allowed \n";
      exit(2);
    };
  };  
  sort(gpio_lines.begin(),gpio_lines.end());
  for (uint32_t i1=0;i1<gpio_lines.size()-1;i1++) {
    if (gpio_lines[i1]==gpio_lines[i1+1]) {
      std::cerr << "GPIO lines must be distinct \n";
      exit(1);
    };
  };


  m_db_iface=db_iface;
  m_unit=unit;
  m_enable_hmc=enable_hmc;
  m_data_in_hmc=data_in_hmc;
  m_clk_hmc=clk_hmc;
  m_data_out_hmc=data_out_hmc;
  m_reset_hmc=reset_hmc;
  m_chip_addr=chip_addr;

  mask_write=(1<<m_enable_hmc) | (1<<m_data_out_hmc) | (1<<m_clk_hmc) | 
    (m_reset_hmc);
  mask_read=(1<<m_data_in_hmc) | (1<<m_dummy);
  mask_all=mask_write | mask_read;


  m_db_iface->set_pin_ctrl(m_unit,0,mask_all);
  m_db_iface->set_gpio_ddr(m_unit,0,mask_read);
  m_db_iface->set_gpio_ddr(m_unit,mask_write,mask_write);
  
  // Set enable high and CLK low
  m_db_iface->set_gpio_out(m_unit,1 << m_enable_hmc,1 << m_enable_hmc ); 
  m_db_iface->set_gpio_out(m_unit,0,1 << m_clk_hmc); 

  /*
  while (1) {

  m_db_iface->set_gpio_out(m_unit,1 << m_enable_hmc,1 << m_enable_hmc ); 
  usleep(1e3);
  m_db_iface->set_gpio_out(m_unit,0,1 << m_enable_hmc ); 
  usleep(1e3);


  };
  */


  // Let reset go high
  m_db_iface->set_gpio_out(m_unit,1<< m_reset_hmc,1 << m_reset_hmc); 
  usleep(2e5);
  m_db_iface->set_gpio_out(m_unit,0,1 << m_reset_hmc); 

};
  
void board_60GHz_base::write_row(uint16_t row_num, uint32_t value) {

  uint32_t total_write;
  int16_t data;
  double T=1000; // Use to increase clock period
  
  //uint32_t value_brs; // Bit reversed value
  uint32_t row_num_brs; // Bit reversed row number

  row_num=row_num & 0xF;
  value=value & 0xFF;

  row_num_brs=0;
  for (int i1=0;i1<6;i1++)
    row_num_brs=row_num_brs | (((row_num>>i1) & 1) << (5-i1));

  //value_brs=0;
  //for (int i1=0;i1<8;i1++)
  //  value_brs=value_brs | (((value >>i1) & 1) << (7-i1));

  //total_write=(1<<14) | (0<<15) | (1<<16) | (1<<17); // R/W +chip address
  //total_write=(1<<14) | (1<<15) | (1<<16) | (1<<17); // R/W +chip address
  total_write=(1<<14) | (m_chip_addr<<15);

  total_write=total_write | value;
  total_write=total_write | (row_num << 8);


  // Set enable low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_enable_hmc);
  usleep(T/2);

  for (int i1=0;i1<=17;i1++) {
      //std::cout << "i1=" << i1 << std::endl;
      // Clock low
      m_db_iface->set_gpio_out(m_unit,0,1 << m_clk_hmc);
      // Write data
      data=(total_write >> i1) & 1;
      //std::cout << "data=" << (data << m_data_sky) << std::endl;
      m_db_iface->set_gpio_out(m_unit,data << m_data_out_hmc,
			       1 << m_data_out_hmc);
      // Dummy read
      m_db_iface->read_gpio(m_unit);
      usleep(T/2);
      // Clock high
      m_db_iface->set_gpio_out(m_unit,1 << m_clk_hmc, 1 << m_clk_hmc);
      // Dummy read
      m_db_iface->read_gpio(m_unit);
      usleep(T/2);

  };
  // Clock low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_clk_hmc);
  // Data  low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_data_out_hmc);
      

  // LE high
  usleep(T/2);
  m_db_iface->set_gpio_out(m_unit,1 << m_enable_hmc,1 << m_enable_hmc);
  usleep(T*100);

};


uint16_t board_60GHz_base::read_row(uint16_t row_num) {
 
   uint16_t value=0;

  uint32_t total_write;
  int16_t data;
  double T=1000; // Use to increase clock period
  
  //uint32_t value_brs; // Bit reversed value
  uint32_t row_num_brs; // Bit reversed row number


  row_num=row_num & 0xF;
  value=value & 0xFF;

  row_num_brs=0;

  for (int i1=0;i1<6;i1++) {
    row_num_brs=row_num_brs | (((row_num>>i1) & 1) << (5-i1));
  };

  
  //value_brs=0;
  //for (int i1=0;i1<8;i1++)
  //  value_brs=value_brs | (((value >>i1) & 1) << (7-i1));


  //total_write=(0<<14) | (0<<15) | (1<<16) | (1<<17); // R/W +chip address
  //total_write=(0<<14) | (1<<15) | (1<<16) | (1<<17); // R/W +chip address
  total_write=(0<<14) | (m_chip_addr<<15);
  total_write=total_write | value;
  total_write=total_write | (row_num << 8);


 
  // Set enable low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_enable_hmc);
  usleep(T/2);

  for (int i1=0;i1<=17;i1++) {
      //std::cout << "i1=" << i1 << std::endl;
      // Clock low
      m_db_iface->set_gpio_out(m_unit,0,1 << m_clk_hmc);
      // Write data
      data=(total_write >> i1) & 1;
      //std::cout << "data=" << (data << m_data_sky) << std::endl;
      m_db_iface->set_gpio_out(m_unit,data << m_data_out_hmc,
			       1 << m_data_out_hmc);
      // Dummy read
      m_db_iface->read_gpio(m_unit);
      usleep(T/2);
      // Clock high
      m_db_iface->set_gpio_out(m_unit,1 << m_clk_hmc, 1 << m_clk_hmc);
      // Dummy read
      m_db_iface->read_gpio(m_unit);
      usleep(T/2);

  };

  // Clock low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_clk_hmc);
  // Data low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_data_out_hmc);
  usleep(T/2);
  // Set enable high
  m_db_iface->set_gpio_out(m_unit,1 << m_enable_hmc,1 << m_enable_hmc);
  usleep(T/2);
  // Clock high
  m_db_iface->set_gpio_out(m_unit,1 << m_clk_hmc, 1 << m_clk_hmc);
  usleep(T/2);
  // Clock low
  m_db_iface->set_gpio_out(m_unit,0, 1 << m_clk_hmc);
  usleep(T/2*3);
  // Enable low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_enable_hmc);
  usleep(T/2);     

  uint32_t temp;
  for (int i1=0;i1<8;i1++) {
    // Clock high
    m_db_iface->set_gpio_out(m_unit,1 << m_clk_hmc, 1 << m_clk_hmc);
    usleep(T/2);
    temp=m_db_iface->read_gpio(m_unit); 
    temp=(temp >> m_data_in_hmc) & 1;
    if (temp==1)
      temp=0;
    else
      temp=1;
    value=value | (temp << i1);
    // Clock low
    m_db_iface->set_gpio_out(m_unit,0, 1 << m_clk_hmc);
    usleep(T/2);
  };
  

  // LE high
  usleep(T);
  m_db_iface->set_gpio_out(m_unit,1 << m_enable_hmc,1 << m_enable_hmc);

  return value;

};



