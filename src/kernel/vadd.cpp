#include <sodium.h>

extern "C" {
	int counter = 0;
	void attest(
	        const unsigned int *in, // Read-Only Vector
	        unsigned int *out,       // Output Result
	        int size                 // Size in integer
	        )
	{
#pragma HLS INTERFACE m_axi port=in bundle=aximm1
#pragma HLS INTERFACE m_axi port=out bundle=aximm1

	if (sodium_init() < 0) {
        /* panic! the library couldn't be initialized; it is not safe to use */
		return;
    }

	    for(int i = 0; i < size; ++i)
	    {
		// TODO: implement
	        // out[i] = in1[i] + in2[i];
	    }
	}
}

