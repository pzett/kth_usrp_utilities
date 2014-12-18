//
// Copyright 2014 Per Zetterberg, KTH.
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


#include "board_60GHz.hpp"
#include <iostream>



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


#define DATA_OUT_HMC 1
#define DATA_IN_HMC 0
#define CLK_HMC 2
#define ENABLE_HMC 3
#define RESET_HMC 4


board_60GHz_TX::board_60GHz_TX(uhd::usrp::dboard_iface::sptr db_iface,
double clock_freq):
board_60GHz_base(db_iface,uhd::usrp::dboard_iface::UNIT_TX,
				   ENABLE_HMC, DATA_IN_HMC, CLK_HMC,
		 DATA_OUT_HMC, RESET_HMC,2+4) {


    m_clock_is_285MHz=(abs(clock_freq-285.714)<abs(clock_freq-308.5714));


    write_row(0,0); // Power on everything
    write_row(1,0); // Power on and highest Q of filter.
    write_row(2,240); // Taken from PC app.
    write_row(3,31); // Taken from PC app.
    write_row(4,63); // Normal operation
    write_row(5,244); // Normal operation
    write_row(6,143); // 

    int l_tx_gain=13; // 0:13. Increasing gain.
    write_row(7,15+16*(13-l_tx_gain)); 
                                   // Highest gain + normal operation
    
    write_row(8,191); // normal operation
    write_row(9,111); // normal operation


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

    // Table 10. 308.5714MHz Reference
    // Frequency(GHz)    DIVRATIO            BAND

    /*   

     57.24                 10101             001
     57.78                 10100             001
     58.32                 10011             010
     58.86                 10010             010
     59.40                 10001             011
     59.94                 10000             011
     60.48                 11111             100
     61.02                 00000             100
     61.56                 00001             101
     62.10                 00010             101
     62.64                 00011             110
     63.18                 00100             110
     63.72                 00101             111 */


    if (m_clock_is_285MHz) { // 60GHz center frequency
      write_row(10,240); // 240+DIVRATIO<4>
      write_row(11,16*(1+2+4)+2*3+1); // 16*DIVRATIO<3:0>+2*BAND+1
    } else { // 60.48GHz center frequency 
      write_row(10,240+1); // 240+DIVRATIO<4>
      write_row(11,16*(1+2+4+8)+2*4+1); // 16*DIVRATIO<3:0>+2*BAND+1
    };



    write_row(12,95); // Syntesizer parameters (lock window)
    write_row(13,128); // normal operation (synths on)
    write_row(14,118); // normal operation

    #if 0
    for (int i1=0;i1<15;i1++) {
      std::cout << "reg=" << i1 << " value=" << read_row(i1) 
          << "\n";
    };
    #endif

    std::cout << "Waiting for PLL lock \n";
    int lock=read_row(15)>> 6;
    while (lock!=1) {
      std::cout << ".";
      usleep(1e6);
      lock=read_row(15)>> 6;
    };
    std::cout << "PLL has locked! \n";
   
}

void board_60GHz_TX::set_gain(uint16_t tx_gain) {
  if (tx_gain>13)
    tx_gain=13;



 
  write_row(7,15+16*(13-tx_gain)); 

};




board_60GHz_RX::board_60GHz_RX(uhd::usrp::dboard_iface::sptr db_iface,
double clock_freq) :
board_60GHz_base(db_iface,uhd::usrp::dboard_iface::UNIT_RX,
				   ENABLE_HMC, DATA_IN_HMC, CLK_HMC,
		 DATA_OUT_HMC, RESET_HMC,1+2+4) {


    
    m_clock_is_285MHz=(abs(clock_freq-285.714)<abs(clock_freq-308.5714));

    write_row(0,128); // Everthing on except ASK mod.
    int bb_gain1=0; // 0-3
    int bb_gain2=0; // 0-3
    int bb_att1=3-bb_gain1;
    int bb_att2=3-bb_gain2;
    write_row(1,bb_att2+4*bb_att1); // Power on + gain control

    int bb_gain_fineI=0; // 0-5
    int bb_gain_fineQ=0; // 0-5

    int bb_att_fineI=5-bb_gain_fineI;
    int bb_att_fineQ=5-bb_gain_fineQ;


    write_row(2,4*bb_att_fineQ+32*bb_att_fineI); 
                                               // Normal operation

    int bb_low_pass_corner=3; // 0=>1.4GHz, 1=>500MHz, 2=> 300MHz, 3=>200MHz.
    int bb_high_pass_corner=2; // 0=>30kHz, 1=>300kHz, 2=>1.5MHz.

    write_row(3,3+16*bb_high_pass_corner+64*bb_low_pass_corner);
                                            // Normal operation
    
    write_row(4,158); // Normal operation

    int if_gain=15; // 0-15
    int if_att=15-if_gain;

    write_row(5,15+16*if_att); // Normal operation
    write_row(6,191); // Normal operation
    write_row(7,109); // Normal operation
    write_row(8,128); // Normal operation
    write_row(9,0); // Normal operation

    if (m_clock_is_285MHz) { // 60GHz center frequency
      write_row(10,240); // 240+DIVRATIO<4>
      write_row(11,16*(1+2+4)+2*3+1); // 16*DIVRATIO<3:0>+2*BAND+1
    } else { // 60.48GHz center frequency 
      write_row(10,240+1); // 240+DIVRATIO<4>
      write_row(11,16*(1+2+4+8)+2*4+1); // 16*DIVRATIO<3:0>+2*BAND+1
    };



    write_row(12,95); // Normal operation
    write_row(13,128); // Normal operation
    write_row(14,118); // Normal operation

    
    #if 0
    for (int i1=0;i1<15;i1++) {
      std::cout << "reg=" << i1 << " value=" << read_row(i1) 
          << "\n";
    };
    #endif

    std::cout << "Waiting for PLL lock \n";
    int lock=read_row(15)>> 6;
    while (lock!=1) {
      std::cout << ".";
      usleep(1e6);
      lock=read_row(15)>> 6;
    };
    std::cout << "PLL has locked! \n";


  


}

void board_60GHz_RX::set_gain(uint16_t rx_gain) {
  if (rx_gain>15)
    rx_gain=15;

   int if_att=15-rx_gain;
   write_row(5,15+16*if_att); // Normal operation

};


void board_60GHz_RX::set_freq(double freq) {

    double freq_actual=freq;

    if (freq<57e9) {
      freq_actual=57e9;
    };
    if (freq>64e9) {
      freq_actual=64e9;
    };


    uint16_t DIVRATIO;
    uint16_t BAND;

    if (m_clock_is_285MHz) {
      DIVRATIO=round((freq_actual-57e9)/0.5e9)+1;
      freq_actual=57e9+0.5e9*(DIVRATIO-1);
       double extra_offset=0.5e9; // Added to make it work. Different from datasheet!
       BAND=floor((freq_actual+extra_offset-57e9)/1e9);
    } else {
      if (freq_actual<60.21e9) {
	DIVRATIO=16+round((59.94e9-freq_actual)/0.54e9);
        if (DIVRATIO<16)
          DIVRATIO=16;
      } else if (freq_actual>60.75e9) {

	DIVRATIO=round((freq_actual-61.02e9)/0.54e9);
	if (DIVRATIO>5)
	  DIVRATIO=5;
      } else
	DIVRATIO=31;

      if (freq<57.24e9) {
         freq_actual=57.24e9;
      };



       BAND=floor((freq_actual-57.24e9)/1.08e9);
       if (BAND>7)
	 BAND=7;

    };


    /*   

     57.24                 10101             001
     57.78                 10100             001
     58.32                 10011             010
     58.86                 10010             010
     59.40                 10001             011
     59.94                 10000             011
     60.48                 11111             100
     61.02                 00000             100
     61.56                 00001             101
     62.10                 00010             101
     62.64                 00011             110
     63.18                 00100             110
     63.72                 00101             111 */





    /*
    std::cout << "DIVRATIO<3:0>=" <<  (DIVRATIO & 15) << "\n";
    std::cout << "BAND=" <<  BAND << "\n";
    std::cout << "240+DIVRATIO<4>=" << 240+(DIVRATIO>>4) << "\n" ;
    std::cout << "16*DIVRATIO<3:0>+2*BAND+1=" << 16*(DIVRATIO & 15)+2*BAND+1 << "\n";*/

    write_row(10,240+(DIVRATIO>>4)); // 240+DIVRATIO<4>
    write_row(11,16*(DIVRATIO & 15)+2*BAND+1); // 16*DIVRATIO<3:0>+2*BAND+1

    std::cout << "60GHz RX board: Target RF Freq=" << freq/1e9 << "GHz \n";
    std::cout << "60GHz RX board: Actual RF Freq=" << freq_actual/1e9 << "GHz \n";


    std::cout << "Waiting for PLL lock \n";
    int lock=read_row(15)>> 6;
    while (lock!=1) {
      std::cout << ".";
      usleep(1e6);
      lock=read_row(15)>> 6;
    };
    std::cout << "PLL has locked! \n";

}

void board_60GHz_TX::set_freq(double freq) {

    double freq_actual=freq;

    if (freq<57e9) {
      freq_actual=57e9;
    };
    if (freq>64e9) {
      freq_actual=64e9;
    };


    uint16_t DIVRATIO;
    uint16_t BAND;

    if (m_clock_is_285MHz) {
       DIVRATIO=round((freq_actual-57e9)/0.5e9)+1;
      freq_actual=57e9+0.5e9*(DIVRATIO-1);
       double extra_offset=0.5e9; // Added to make it work. Different from datasheet!
       BAND=floor((freq_actual+extra_offset-57e9)/1e9);
    } else {
      if (freq_actual<60.21e9) {
	DIVRATIO=16+round((59.94e9-freq_actual)/0.54e9);
        if (DIVRATIO<16)
          DIVRATIO=16;
      } else if (freq_actual>60.75e9) {

	DIVRATIO=round((freq_actual-61.02e9)/0.54e9);
	if (DIVRATIO>5)
	  DIVRATIO=5;
      } else
	DIVRATIO=31;

      if (freq<57.24e9) {
         freq_actual=57.24e9;
      };

       BAND=floor((freq_actual-57.24e9)/1.08e9);
       if (BAND>7)
	 BAND=7;

    };




    write_row(10,240+(DIVRATIO>>4)); // 240+DIVRATIO<4>
    write_row(11,16*(DIVRATIO & 15)+2*BAND+1); // 16*DIVRATIO<3:0>+2*BAND+1

    std::cout << "60GHz TX board: Target RF Freq=" << freq/1e9 << "GHz \n";
    std::cout << "60GHz TX board: Actual RF Freq=" << freq_actual/1e9 << "GHz \n";


    std::cout << "Waiting for PLL lock \n";
    int lock=read_row(15)>> 6;
    while (lock!=1) {
      std::cout << ".";
      usleep(1e6);
      lock=read_row(15)>> 6;
    };
    std::cout << "PLL has locked! \n";

}
