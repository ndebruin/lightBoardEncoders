#pragma once

#include <Arduino.h>
// #include <vector>

#include "Parameter.h"

#define STORAGE_LENGTH 128 // this is like probably overkill but just define it based off of left over SRAM

class DataStorage
{
    public:
        DataStorage(){}

        void clearChannel()
        {
            channelValue = 0.0;
            channelSelection = "";
        }

        void setChannel(String ChannelSelection, float ChannelValue)
        {
            channelValue =  ChannelValue;
            channelSelection = ChannelSelection;
        }

        void removeParam(int32_t eosIndex)
        {
            params2[eosIndex-1] = nullParam;
            paramsSize--;
        }

        bool addParam(Parameter param)
        {  
            if(param.index-1 >= STORAGE_LENGTH){return false;} // if we overload the storage array then we return false

            params2[param.index-1] = param;
            paramsSize++;
            return true;
        }

        bool addParam(int32_t index, String name, int32_t category, float value)
        {
            Parameter param;
            param.index = index;
            param.name = name;
            param.category = category;
            param.value = value;
            
            return addParam(param);
        }

        uint getParamCount(){ return paramsSize; };

        Parameter getParam(uint16_t index){ return params2[index]; };

        void setParamValue(uint16_t index, float value){ params2[index].value = value; };

        String getChannelSelection(){ return channelSelection; };
        float getChannelValue(){ return channelValue; };

        /// @brief Find a parameter in our storage based on name.
        /// @param paramIndex ETC Eos index to find.
        /// @return Returns vector index of parameter if found. Returns -1 otherwise.
        int find(int32_t paramIndex)
        {
            // basic iterator that works in O(N) time which is fine
            for(uint16_t i=0; i < paramsSize; i++){
                if(params2[i].index == paramIndex){
                    return i;
                }
            }
            return -1; // not found
        }


    private:
        Parameter params2[STORAGE_LENGTH];
        uint16_t paramsSize =0;

        // an empty value has an index and category of -1. This shouldn't be matched from any real eOS output.
        Parameter nullParam = {-1, "", -1, 0.0};
        
        float channelValue;
        String channelSelection;
};