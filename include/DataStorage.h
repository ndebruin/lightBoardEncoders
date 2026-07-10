#pragma once

#include <Arduino.h>
// #include <vector>

#include "Parameter.h"

// this is like probably overkill but just define it based off of left over SRAM
#define STORAGE_LENGTH 128

class DataStorage
{
    public:
        DataStorage(){}

        void clearChannel()
        {
            channelValue = 0.0;
            channelSelection = "";
        };

        void setChannel(String ChannelSelection, float ChannelValue)
        {
            channelValue =  ChannelValue;
            channelSelection = ChannelSelection;
        };

        void removeParam(int32_t eosIndex)
        {
            categoryCount[((params[eosIndex-1].category)-1)]--;
            params[eosIndex-1] = nullParam;
            paramsSize--;
        };

        bool addParam(Parameter param)
        {  
            if(param.index-1 >= STORAGE_LENGTH){return false;} // if we overload the storage array then we return false

            params[param.index-1] = param;
            paramsSize++;
            categoryCount[(param.category-1)]++;
            return true;
        };

        bool addParam(int32_t index, String name, Category category, float value)
        {
            Parameter param;
            param.index = index;
            param.name = name;
            param.category = category;
            param.value = value;
            
            return addParam(param);
        };

        uint16_t getParamCount(){ return paramsSize; };

        uint16_t getCategoryParamCount(Category category){ return categoryCount[category-1]; };

        Parameter getParam(int16_t index)
        { 
            if(index == -1){return nullParam;};
            return params[index]; 
        };

        void setParamValue(uint16_t index, float value){ params[index].value = value; };

        String getChannelSelection(){ return channelSelection; };
        float getChannelValue(){ return channelValue; };

        /// @brief Find a parameter in our storage based on name.
        /// @param paramIndex ETC Eos index to find.
        /// @return Returns vector index of parameter if found. Returns -1 otherwise.
        int find(int32_t paramIndex)
        {
            // basic iterator that works in O(N) time which is fine
            for(uint16_t i=0; i < paramsSize; i++){
                if(params[i].index == paramIndex){
                    return i;
                }
            }
            return -1; // not found
        };

        int16_t find(Category category, int32_t categoryIndex)
        {
            uint32_t categoryParamCount = 0;
            // basic iterator that works in O(N) time which is fine
            for(uint16_t i=0; i < paramsSize; i++){
                if(params[i].category == category){
                    if(categoryParamCount == (uint32_t)categoryIndex){
                        return i;
                    }
                    categoryParamCount++;
                }
            }
            return -1; // not found
        };

    private:
        Parameter params[STORAGE_LENGTH];
        uint16_t paramsSize = 0;
        uint16_t categoryCount[6];

        // an empty value has an index of -1 and category of None. This shouldn't be matched from any real eOS output.
        Parameter nullParam = {-1, "", Category::None, 0.0};
        
        float channelValue;
        String channelSelection;
};