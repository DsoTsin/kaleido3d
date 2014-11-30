#pragma once

#include "CommandEncoder.hpp"

struct ComputeCommandEncoder : public CommandEncoder {

    void dispatchThreadgroups();

    void endEncoding() override;

};