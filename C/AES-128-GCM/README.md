AES-128-GCM
============

This is a pretty basic implementation of AES-128-GCM i did for a university project. There are probably multiple
improvements that could be made but time is never enough.
The requirements were:

Implement the AES-128 block cipher encryption function. The official specifications of AES are available in [FIPS
Publication 197](http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf).

Implement the Galois counter mode (GCM) mode for AES-128 block cipher after that.
The GMC specifications are in [NIST 800-38D](http://csrc.nist.gov/publications/nistpubs/800-38D/SP-800-38D.pdf).

The following simplifications apply:

1. The length of the IV is fixed to 12 bytes (96 bits).
2. The length of the plaintext is always a multiple of the block size (16 bytes). Hence, you don't have to process any final partial blocks.
3. The length of the associated data is also always a multiple of the block size (16 bytes).
4. The lenght of the tag is one block (16 bytes).

##License

This implementation is licensed under the [Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported](https://creativecommons.org/licenses/by-nc-sa/3.0/)
