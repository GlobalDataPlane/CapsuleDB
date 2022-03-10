#ifndef FAKECAPSULE_H
#define FAKECAPSULE_H

#include <string>
#include "capsuleBlock.hh"
#include "src/shared/capsule.h"

void sha256_string(const char *string, char outputBuffer[65]);

std::string putCapsuleBlock(CapsuleBlock inputBlock);

CapsuleBlock getCapsuleBlock(std::string inputHash);

#endif