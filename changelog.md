## 2.0.0 - 2016/11/
-- Fix protocols SHARP , changed data to LONG LONG to allow return 48 bits 
-- Fix SONY  and included repeatsee MIN_REPEAT and now decoding command and address 
-- Fix NEC and implement check integrity 
-- Fix SHARP 
-- Fix SANYO 
-- RC5/RC6 Clean up Debug and include VERBOSE .
-- PANASONIC 48 bits works now  check long long data Rawdata 
, SONY, NEC, MITSUBISH and review the code 
-- Implemented LG 32 bits and changed SAMSUNG to have the right decode 
-- Changed to to take all times to allow check repeat times 
-- Implemented allow change dutycycle
-- changed all debug messages DEBUG to behavior as error to have a clear output 
-- Included dump when DEBUG 
-- JVC included the loop as default see MIN_REPEAT and changed signature to (unsigned long data,  int nbits)  
-- SONY  included the loop as default see MIN_REPEAT 
-- Include the address and comand at decode process to all protocos we know . 
-- Changed Rawdata decode to long long to allow decode larger numbers  
-- Split files and take changes from z30t0 
-- Changed USECPERTICK  to 1 
-- Created generic functions to be used at client to allow no changes when implementing new protocols 
-- Please use it ....
--   bool send_raw    (String protocol, long long rawData, int IrBits) ; 
--   bool send_raw    (int id         , long long rawData, int bits); 
--   bool send_address(String protocol, int address       , int command); 
--   bool send_address(integer id     , int address       , int command);  

