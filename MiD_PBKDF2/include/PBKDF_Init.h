/*
*	=====================================
*	Copyright (c) El Mostafa IDRASSI 2017
*	mostafa.idrassi@tutanota.com
*	Apache License
*	=====================================
*/

#include "Hmac_PRF.h"

 int PBKDF2(Hmac_PRF& p, const size_t & iterationCount, const unsigned char password[], const unsigned int & passwordLength, const unsigned char salt[], const unsigned int & saltSize, unsigned char dKey[], const unsigned int & dkeyLength);

/**
*	Validates the PBKDF2 implementation
*	Based on RFC 6070 https://www.ietf.org/rfc/rfc6070.txt
*	Only test vectors available are for HMAC-SHA1
*	The 4th test in RFC 6070 has been commented out because of the amount of time it
*	takes to finish (due to the IterationCount being equal to 16777216)
*/
 int PBKDF2_Init();