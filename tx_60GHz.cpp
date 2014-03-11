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

namespace po = boost::program_options;


class board_60GHz_base {

  public:  

  board_60GHz_base(uhd::usrp::dboard_iface::sptr db_iface,
		      uhd::usrp::dboard_iface::unit_t unit,
		 uint16_t enable_hmc, uint16_t data_in_hmc,
		 uint16_t clk_hmc, uint16_t data_out_hmc, uint16_t reset_hmc);

  void write_row(uint16_t row_num, uint32_t value);
  uint16_t read_row(uint16_t row_num);


  private:

  uhd::usrp::dboard_iface::aux_dac_t m_which_dac;
  
  uint16_t m_enable_hmc, m_data_in_hmc, m_data_out_hmc;
  uint16_t m_clk_hmc, m_reset_hmc, m_dummy;
  uint16_t mask_read, mask_write, mask_all;

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
        ("txrate", po::value<double>(&tx_rate)->default_value(100e6/16), "rate of outgoing samples")
        ("freq", po::value<double>(&freq)->default_value(0), "rf center frequency in Hz")
        ("LOoffset", po::value<double>(&LOoffset)->default_value(0), "Offset between main LO and center frequency")
        ("forever",po::value<bool>(&forever)->default_value(false), "run indefinetly")
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
    board_60GHz_base my_60GHz_board(db_iface,uhd::usrp::dboard_iface::UNIT_TX,
				  7,6,5,4,3);


    my_60GHz_board.write_row(0,0); // Power everything on
    my_60GHz_board.write_row(1,0); // Highest Q and everything on
    my_60GHz_board.write_row(2,0xF0); // Normal operation
    my_60GHz_board.write_row(3,0x1F); // Normal operation
    my_60GHz_board.write_row(4,0x3F); // Normal operation
    my_60GHz_board.write_row(5,0x04); // Normal operation
    my_60GHz_board.write_row(6,0x8F); // Normal operation

    my_60GHz_board.write_row(7,0x0F); // Highest gain + normal operation
    // my_60GHz_board.write_row(7,0xDF); // Lowest gain + normal operation
    my_60GHz_board.write_row(8,0xBF); // normal operation
    my_60GHz_board.write_row(9,0x6F); // normal operation
    my_60GHz_board.write_row(10,0xF0); // normal operation @ 285.7143MHz ref.
    my_60GHz_board.write_row(11,0x77); // 60GHz carrier frequency 0111 011 0
    my_60GHz_board.write_row(12,0x5F); // Syntesizer parameters
    my_60GHz_board.write_row(13,0x80); // normal operation (synths on)
    my_60GHz_board.write_row(14,0x76); // normal operation
    

    for (int i1=0;i1<1000;i1++) {
      std::cout << "Lock state=" << (my_60GHz_board.read_row(15) >>4) 
		<< " (01=lock)" << "\n";
      usleep(1e6);
    };

    //dev->set_tx_bandwidth(36e6);


    std::cout << "freq=" << freq << "\n";
    std::cout << "LOoffset=" << LOoffset << "\n";
    

    uhd::tune_result_t tr;
    uhd::tune_request_t trq(freq,LOoffset); //std::min(tx_rate,10e6));
    tr=dev->set_tx_freq(trq,0);
    

    dev->set_tx_gain(gain);
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
	 uint16_t clk_hmc, uint16_t data_out_hmc, uint16_t reset_hmc)
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

  mask_write=(1<<m_enable_hmc) && (1<<m_data_in_hmc) && (1<<m_clk_hmc) && 
    (m_reset_hmc);
  mask_read=(1<<m_data_in_hmc) || (1<<m_dummy);
  mask_all=mask_write || mask_read;
  

  m_db_iface->set_gpio_out(m_unit,1,1 << m_enable_hmc); // Set enable high
  m_db_iface->set_gpio_out(m_unit,0,1 << m_clk_hmc); // Set CLK low


};
  
void board_60GHz_base::write_row(uint16_t row_num, uint32_t value) {

  uint32_t total_write;
  int16_t data;
  double T=50; // Use to increase clock period
  
  uint32_t value_brs; // Bit reversed value
  uint32_t row_num_brs; // Bit reversed row number

  row_num=row_num & 0xF;
  value=value & 0xFF;

  row_num_brs=0;
  for (int i1=0;i1<6;i1++)
    row_num_brs=row_num_brs || (((row_num>>i1) && 1) << (5-i1));

  value_brs=0;
  for (int i1=0;i1<8;i1++)
    value_brs=value_brs || (((value >>i1) && 1) << (7-i1));


  total_write=(1<<14) || (0<<15) || (1<<16) || (1<<17); // R/W +chip address
  total_write=total_write || value_brs;
  total_write=total_write || (row_num_brs>>8);


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
  

};


uint16_t board_60GHz_base::read_row(uint16_t row_num) {
 
   uint16_t value=0;

  uint32_t total_write;
  int16_t data;
  double T=50; // Use to increase clock period
  
  uint32_t value_brs; // Bit reversed value
  uint32_t row_num_brs; // Bit reversed row number

  row_num=row_num & 0xF;
  value=value & 0xFF;

  row_num_brs=0;
  for (int i1=0;i1<6;i1++)
    row_num_brs=row_num_brs || (((row_num>>i1) && 1) << (5-i1));

  value_brs=0;
  for (int i1=0;i1<8;i1++)
    value_brs=value_brs || (((value >>i1) && 1) << (7-i1));


  total_write=(0<<14) || (0<<15) || (1<<16) || (1<<17); // R/W +chip address
  total_write=total_write || value_brs;
  total_write=total_write || (row_num_brs>>8);


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
  usleep(T/2);
  // Enable low
  m_db_iface->set_gpio_out(m_unit,0,1 << m_enable_hmc);
  usleep(T/2);     

  uint32_t temp;
  for (int i1=0;i1<8;i1++) {
    // Clock high
    m_db_iface->set_gpio_out(m_unit,1 << m_clk_hmc, 1 << m_clk_hmc);
    usleep(T/2);
    temp=m_db_iface->read_gpio(m_unit); 
    temp=(temp >> m_data_out_hmc) & 1;
    value=value || (temp << i1);
    // Clock low
    m_db_iface->set_gpio_out(m_unit,0, 1 << m_clk_hmc);
    usleep(T/2);
  };
  

  // LE high
  usleep(T);
  m_db_iface->set_gpio_out(m_unit,1 << m_enable_hmc,1 << m_enable_hmc);

  return value;

};


#if 0

  void read_row(uint16_t row_num, uint32_t value);


class tx_board_60GHz {

  public:  

  tx_board_60GHz(uhd::usrp::dboard_iface::sptr db_iface,
		      uhd::usrp::dboard_iface::unit_t unit,
		 uint16_t enable_hmc, uint16_t data_in_hmc,
		 uint16_t clk_hmc, uint16_t data_out_hmc, uint16_t reset_hmc);

  void write_row(uint16_t row_num, uint32_t value);
  void read_row(uint16_t row_num, uint32_t value);


  private:

  uhd::usrp::dboard_iface::aux_dac_t m_which_dac;
  
  uint16_t m_enable_hmc, m_data_in_hmc, m_data_out_hmc;
  uint16_t m_clk_hmc, m_reset_hmc;

  uhd::usrp::dboard_iface::sptr m_db_iface;
  uhd::usrp::dboard_iface::unit_t m_unit;
 
};
#endif

