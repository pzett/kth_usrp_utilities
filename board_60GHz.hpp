//
// Copyright 2014 Modified by Per Zetterberg, KTH.
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


#ifndef BOARD_60GHZ_HPP
#define BOARD_60GHZ_HPP

#include <uhd/usrp/multi_usrp.hpp>


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


class board_60GHz_TX : public board_60GHz_base {
   public:   
   board_60GHz_TX(uhd::usrp::dboard_iface::sptr db_iface, 
		  double clock_freq=285.714);
   /* Set gain between 0 and 13. Steps are 1.3dB. */
   void set_gain(uint16_t gain);
   void set_freq(double freq);
   bool m_clock_is_285MHz;
}; 


class board_60GHz_RX : public board_60GHz_base {
   public:   
    board_60GHz_RX(uhd::usrp::dboard_iface::sptr db_iface,
   double clock_freq=285.714);
   /* Set gain between 0 and 15. Steps are 1dB. */
   void set_gain(uint16_t gain);
   void set_freq(double freq);
   bool m_clock_is_285MHz;
}; 


#endif

