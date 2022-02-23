# AES_LibExample
Dialog based example of AES encrytption/decryption that uses AES Library for AES128/192/256 in ECB, CBC, CTR, OFB and CFB modes

This dektop application is a sample that uses AES Library; https://github.com/tanel312/AES_Library 
Input data is read from a file and encryted or decrypted in accordance with the type and mode selection. The output needs to be saved to a file manually.
It supports 128-bit / 192-bit / 256-bit key in ECB, CBC, CTR, OFB and CFB modes. The key can be entered as ASCII characters or Hexadecimal values.
The size of the key is checked but validity of input is not checked. i.e. any character can be entered in hex mode.
It also accepts initial vector (IV) value since it is required for modes except ECB.
IV input needs to be hexadecimal value but there is no input data validation. The size of IV is checked.
The key and IV can be generated automatically by using a simple methodology based and system time and random number generation.
If they are auto generated, they need to be recorded since they will be needed for decryption.
Information or error messages are displayed in the Status field such as input file error or key size error.

It is is provided under GNU General Public License, as it is with no warranty or support. 
