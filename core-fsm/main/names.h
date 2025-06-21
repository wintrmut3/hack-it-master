enum state{
  // PWRON, -- this first startup code after power cycle can be in setup()
  INIT,
  START_SUBGAME,
  WAIT_SUBGAME,
  END_SUBGAME,
  END_ALL_GAMES, // end all games
  WAIT_LDRBRD,
  CLEANUP
};

enum game{
  CART,
  HEXIT,
  CODEIT,
  WIREIT,
  NUM_GAMES
};

