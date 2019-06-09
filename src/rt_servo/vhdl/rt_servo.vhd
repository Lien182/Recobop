library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library reconos_v3_01_a;
use reconos_v3_01_a.reconos_pkg.all;

library rt_servo_v1_00_a;
use rt_servo_v1_00_a.reconos_thread_pkg.all;

entity rt_servo is
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
		HWT_Signal : in  std_logic;
		HWT_MBREAD : out std_logic
	);
end entity rt_servo;

architecture implementation of rt_servo is
	signal i_osif  : i_osif_t;
	signal o_osif  : o_osif_t;
	signal i_memif : i_memif_t;
	signal o_memif : o_memif_t;

	type STATE_TYPE is (STATE_THREAD_INIT,STATE_INIT_DATA,STATE_GET_SERVO_SINK, STATE_GET_DEMONSTRATOR_NR,
	                    STATE_CMD,STATE_STORE);
	signal state : STATE_TYPE;

	subtype C_ANGLE_RANGE is natural range 31 downto 21;
	subtype C_LEG_RANGE   is natural range 20 downto 18;

	signal cmd    : std_logic_vector(31 downto 0);

	signal srv0_a, srv1_a, srv2_a, srv3_a, srv4_a, srv5_a : unsigned(10 downto 0);
	signal srv0_c, srv1_c, srv2_c, srv3_c, srv4_c, srv5_c : unsigned(21 downto 0);
	signal srv0_p, srv1_p, srv2_p, srv3_p, srv4_p, srv5_p : std_logic;

	constant C_SRV0_CAL : integer := 57334;
	constant C_SRV1_CAL : integer := 63222;
	constant C_SRV2_CAL : integer := 53375;
	constant C_SRV3_CAL : integer := 64465;
	constant C_SRV4_CAL : integer := 57482;
	constant C_SRV5_CAL : integer := 66722;

	signal srv_count : unsigned(21 downto 0) := (others => '0');

	signal ignore, ret : std_logic_vector(31 downto 0);
	
	signal rb_info,servo_base_addr, demonstrator_nr : unsigned(31 downto 0);
	
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

			srv0_a <= to_unsigned(900, 11);
			srv1_a <= to_unsigned(900, 11);
			srv2_a <= to_unsigned(900, 11);
			srv3_a <= to_unsigned(900, 11);
			srv4_a <= to_unsigned(900, 11);
			srv5_a <= to_unsigned(900, 11);
			
		elsif rising_edge(HWT_Clk) then
			HWT_MBREAD <= '0';
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
                        state <= STATE_GET_SERVO_SINK;
                     end if;
                        
                when STATE_GET_SERVO_SINK => 
                        memif_read_word(i_memif, o_memif, std_logic_vector(rb_info + 44), ret, done);
                        if done then
                             servo_base_addr <= unsigned(ret);
                             state <= STATE_GET_DEMONSTRATOR_NR;
                        end if;

				when STATE_GET_DEMONSTRATOR_NR => 
                        memif_read_word(i_memif, o_memif, std_logic_vector(rb_info + 48), ret, done);
                        if done then
                             demonstrator_nr <= unsigned(ret);
                             state <= STATE_CMD;
                        end if;			

				when STATE_CMD =>
					osif_mbox_get(i_osif, o_osif, std_logic_vector(demonstrator_nr), cmd, done);

					if done then
						state <= STATE_STORE;
						if cmd(C_LEG_RANGE) = "000" then
							HWT_MBREAD <= '1';
						end if;
					end if;
				
				when STATE_STORE =>
									
					memif_write_word(i_memif, o_memif, std_logic_vector(servo_base_addr(31 downto 5)) & cmd(C_LEG_RANGE) & "00", x"00000" & '0' & cmd(C_ANGLE_RANGE), done);
                    if done then
                        state <= STATE_CMD;
                    end if;
			end case;
		end if;
	end process osfsm_proc;

end architecture;
