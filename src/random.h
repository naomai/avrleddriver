/*
 * random.h
 *
 * Created: 29.10.2019 18:33:52
 *  Author: bun
 */ 


#ifndef RANDOM_H_
#define RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif
	uint8_t random8();
	uint16_t random16();
	uint32_t random32();
	void initRandom() __attribute__((optimize("Os")));
	uint32_t randomFeedEntropy();
	void randomFeedEntropyDword(int32_t entropy);
#ifdef __cplusplus
}
#endif

#define RANDOM_SRAM_ENTROPY_POOL_SIZE 10



#endif /* RANDOM_H_ */