#pragma once
#include "oalDef.h"
#include <memory>

namespace Nix
{
    namespace openal
    {
        struct Context
        {
            ALCcontext * context;
			void MakeCurrent();
			void Suspend();
			void Process();
        };
        
        struct Device
        {
            ALCdevice * device;
			Device();  
			void release();
			std::shared_ptr<Context> CreateContext(const int * _attrlist);
        };

		std::shared_ptr<Device> GetDevice(const char * _name);
    }
}
