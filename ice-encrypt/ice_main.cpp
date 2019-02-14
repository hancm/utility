#include <iostream>
#include "ice.h"

int main(void)
{
	char* pw = "123hancm456";
	unsigned char ptxt[9] = "ha韩长";
	unsigned char ctxt[9] = {0};
{
	ICE_KEY *ice = ice_key_create();
	ice_key_set(ice, (unsigned char*)pw);
	
	ice_key_encrypt(ice, ptxt, ctxt);
	std::cout << "encrypt: " << ctxt << std::endl;
	
	ice_key_decrypt (ice, ctxt, ptxt);
	std::cout << "decrypt: " << ptxt << std::endl;
	
	ice_key_destroy (ice);
}
	return 0;
}