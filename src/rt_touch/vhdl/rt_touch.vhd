library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

library rt_touch_v1_00_a;
use rt_touch_v1_00_a.reconos_thread_pkg.all;

entity rt_touch is
	port (
		-- OSIF FIFO ports
		OSIF_Sw2Hw_Data    : in  std_logic_vector(31 downto 0);
		OSIF_Sw2Hw_Empty   : in  std_logic;
		OSIF_Sw2Hw_RE      : out std_logic;

		OSIF_Hw2Sw_Data    : out std_logic_vector(31 downto 0);
		OSIF_Hw2Sw_Full    : in  std_logic;
		OSIF_Hw2Sw_WE      : out std_logic;

		-- MEMIF FIFO ports
		MEMIF_Hwt2Mem_Data    : out std_logic_vector(31 downto 0);
		MEMIF_Hwt2Mem_Full    : in  std_logic;
		MEMIF_Hwt2Mem_WE      : out std_logic;

		MEMIF_Mem2Hwt_Data    : in  std_logic_vector(31 downto 0);
		MEMIF_Mem2Hwt_Empty   : in  std_logic;
		MEMIF_Mem2Hwt_RE      : out std_logic;

		HWT_Clk    : in  std_logic;
		HWT_Rst    : in  std_logic;
		HWT_Signal : in  std_logic
	);
end entity rt_touch;

architecture implementation of rt_touch is
	signal i_osif  : i_osif_t;
	signal o_osif  : o_osif_t;
	signal i_memif : i_memif_t;
	signal o_memif : o_memif_t;

	type STATE_TYPE is (STATE_THREAD_INIT, STATE_INIT_DATA,STATE_CTRL,STATE_GET_DEMONSTRATOR_NR,
	                    STATE_GET_TOUCH_SOURCE, STATE_READ_X_POS, STATE_CHECK_X_POS, 
	                    STATE_READ_Y_POS, STATE_WAIT,
	                    STATE_STORE_POS, STATE_STORE_DELTA);
	signal state : STATE_TYPE;


	signal rb_info, demonstrator_nr : unsigned(31 downto 0);
	
	signal touch_base_addr : unsigned(31 downto 0);

	signal x_pos_s, y_pos_s     : std_logic_vector(11 downto 0);
    signal x_pos_old, y_pos_old     : std_logic_vector(11 downto 0);

	signal ignore, ret : std_logic_vector(31 downto 0);

    signal cnt_old : std_logic_vector(19 downto 0);
    
    signal cnt : unsigned(31 downto 0);


begin
	osif_setup (
		i_osif,
		o_osif,
		OSIF_Sw2Hw_Data,
		OSIF_Sw2Hw_Empty,
		OSIF_Sw2Hw_RE,
		OSIF_Hw2Sw_Data,
		OSIF_Hw2Sw_Full,
		OSIF_Hw2Sw_WE
	);

	memif_setup (
		i_memif,
		o_memif,
		MEMIF_Mem2Hwt_Data,
		MEMIF_Mem2Hwt_Empty,
		MEMIF_Mem2Hwt_RE,
		MEMIF_Hwt2Mem_Data,
		MEMIF_Hwt2Mem_Full,
		MEMIF_Hwt2Mem_WE
	);

	osfsm_proc: process (HWT_Clk,HWT_Rst,o_osif,o_memif) is
		variable resume, done : boolean;
	begin
		if HWT_Rst = '1' then
			osif_reset(o_osif);
			memif_reset(o_memif);

			state <= STATE_THREAD_INIT;
		elsif rising_edge(HWT_Clk) then

			case state is
				when STATE_THREAD_INIT =>
					osif_read(i_osif, o_osif, ignore, done);
					if done then
						state <= STATE_INIT_DATA;
					end if;

				when STATE_INIT_DATA =>
					osif_get_init_data(i_osif, o_osif, ret, done);
					if done then
						rb_info <= unsigned(ret);
						state <= STATE_GET_DEMONSTRATOR_NR;
					end if;

				
				when STATE_GET_DEMONSTRATOR_NR => 
                        memif_read_word(i_memif, o_memif, std_logic_vector(rb_info + 48), ret, done);
                        if done then
                             demonstrator_nr <= unsigned(ret);
                             state <= STATE_GET_TOUCH_SOURCE;
                        end if;

                when STATE_GET_TOUCH_SOURCE => 
                    memif_read_word(i_memif, o_memif, std_logic_vector(rb_info + 40), ret, done);
                    if done then
                        touch_base_addr <= unsigned(ret);
                        state <= STATE_READ_X_POS;
                    end if;
					
                when STATE_READ_X_POS =>
                    memif_read_word(i_memif, o_memif, std_logic_vector(touch_base_addr), ret, done);
                    if done then
                        state <= STATE_CHECK_X_POS;
                    end if;
                when STATE_CHECK_X_POS =>
                    if cnt_old /= ret(31 downto 12) then
                        --value is new 
                        cnt_old <= ret(31 downto 12);
                        x_pos_s <= ret(11 downto 0);
                        state <= STATE_READ_Y_POS;
                    else
                        state <= STATE_READ_X_POS;
                    end if;
                    
                when STATE_READ_Y_POS => 
                    memif_read_word(i_memif, o_memif, std_logic_vector(touch_base_addr + 4), ret, done);
                    if done then
                        y_pos_s <= ret(11 downto 0);
                        state <= STATE_STORE_POS;
                    end if;                    
                when STATE_STORE_POS =>
                    if x_pos_s /= x_pos_old OR y_pos_s /= y_pos_old then

						osif_mbox_put(i_osif, o_osif, std_logic_vector(demonstrator_nr), x"00" & x_pos_s & y_pos_s, ignore, done);
                        
                        if done then
						    x_pos_old <= x_pos_s;
                        	y_pos_old <= y_pos_s;
                            state <= STATE_WAIT;
                            cnt <= (others => '0');
                        end if;
                        

                         
                    else
                        state <= STATE_WAIT;                        
                    end if;

                when STATE_WAIT =>
                    if(cnt > 1000000) then
                        state <= STATE_READ_X_POS;
                    else
                        cnt <= cnt + 1;
                    end if;

				when others =>

			end case;
		end if;
	end process osfsm_proc;
end architecture;
