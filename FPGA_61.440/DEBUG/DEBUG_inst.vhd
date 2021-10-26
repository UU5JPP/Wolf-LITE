	component DEBUG is
		port (
			probe : in std_logic_vector(11 downto 0) := (others => 'X')  -- probe
		);
	end component DEBUG;

	u0 : component DEBUG
		port map (
			probe => CONNECTED_TO_probe  -- probes.probe
		);

