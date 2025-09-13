#pragma once

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
  HEXIT,
  CODEIT,
  WIREIT,
  CART,
  CATCHIT,
  NUM_GAMES,
  NONE
};

char game2str[7][16] = {"hexit", "codeit", "wireit", "cart", "catchit", "n_gms_inval", "NONE"}; 


const uint8_t cart_address = 0x6F;
const uint8_t codeit_address = 0xC;  // currently not set - temp - C . before was 0x8
const uint8_t leaderbrd_address = 0x10;
const uint8_t wireit_address = 0xA;
const uint8_t hexit_address = 0x9;
const uint8_t catchit_address = 0x13;

uint8_t gameToi2cAddress(game g) {
  switch (g) {
    case CART: return cart_address;
    case CODEIT: return codeit_address;
    case WIREIT: return wireit_address;
    case HEXIT: return hexit_address;
    case CATCHIT: return catchit_address;
    default:
      return 0;
  }
}


game i2cAddressToGame(uint8_t addr){
  switch(addr){
    case cart_address: return CART;
    case codeit_address: return CODEIT;
    case wireit_address: return WIREIT;
    case hexit_address: return HEXIT;
    case catchit_address: return CATCHIT;
    default: return NONE;
  }
}