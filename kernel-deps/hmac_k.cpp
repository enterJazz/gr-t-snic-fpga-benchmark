#include "hmac_k.h"
#include <stdio.h>

extern "C" {
void sign_rsa_k(uint8_t* h, uint8_t* output) {
#ifdef PRINT_DEBUG
	printf("%s\n", __func__);
#endif
	helper(h, output);
}
}
