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

	    for(int i = 0; i < size; ++i)
	    {
		// TODO: implement
	        out[i] = in[i];
	    }
	}
}

