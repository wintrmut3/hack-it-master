# How to run HackIt standalone

**Note 1**: I have already switched over the code in code-it.ino, hex-it.ino, cart.ino and wire-it.ino to run in their standalone modes. You can see a description below of how these games are meant to be played.

## How to program

```
git clone https://github.com/wintrmut3/hack-it-master.git
git checkout jzx/autostart
```
Go into hex-it.ino, cart.ino, wire-it.ino and reprogram the respective arduino unos attached to the subgames. THey should work powered off USB.

Code it / code-it.ino runs on ESP32, can be targeted by downloading ESP32 board from Arduino board manager, and targeting board esp32 devkit v1. Please be careful with this subgame / don't use it because **there are very delicate wires** on the back.

## Subgame Descriptions

- CART/Click-It - Click at the right time - A timing challenge where players need to stop an LED moving around a ring at a designated spot, by clicking a button. There are no lives or spam protection.
- Wire-It - A perception challenge where players need to play as a switchboard operator. Players must connect the left column of 4 RGB LEDs to the right column by making wired connections between equal colors. There may not be enough banana plug cables, but you can use aligator clips as well by clipping the lip of the port.
- Code-It - A memory challenge where a short password is flashed on the screen and the player must input it on a keypad by memorizing.
- Hex-It - A math challenge where a hex value is showed on a 7seg and the player must translate it to binary by setting switches to nmatch the binary repr of the displayed hex. The least significant bit (2^0=1) is on the right, where the greatest significant bit is on the left. The player should press the button when they think they are done. This game is in a TESTING environment where the actual value of the switches is translated to hex and displayed on the screen, which makes gameplay a lot easier.


**Note 2**: I have put in AUTOSTART/TESTING variables to bypass the normal gameplay loop on this branch. Usually subgames won't start until they get a signal from the master controller to do so, but with `#define AUTOSTART` or `#define TESTING` they take another game-dependent input, like a button press, as the start signal to transition from IDLE to the game playing state. In some cases, the transition is automatic.
