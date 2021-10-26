	component DEBUG2 is
		port (
			probe : in std_logic_vector(23 downto 0) := (others => 'X')  -- probe
		);
	end component DEBUG2;

	u0 : component DEBUG2
		port map (
			probe => CONNECTED_TO_probe  -- probes.probe
		);

