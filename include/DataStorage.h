#pragma once

#include <Arduino.h>
#include <vector>

#include "Util.h"

class DataStorage
{
    public:
        DataStorage(){}

        void clear()
        {
            channelValue = 0.0;
            channelSelection = "";
            // params.clear();
        }

        void setChannel(String ChannelSelection, float ChannelValue)
        {
            channelValue =  ChannelValue;
            channelSelection = ChannelSelection;
        }

        void removeParam(int32_t eosIndex)
        {
            int index = find(eosIndex);
            if(index == -1){return;};
            params.erase(params.begin()+index);
        }

        void clearFromParam(size_t vectorIndex)
        {
            params.erase(params.begin()+vectorIndex, params.end());
        }
        
        // unsure if this works tbh
        void addParam(Parameter param)
        {
            params.push_back(param);
        }

        void addParam(int32_t index, String name, int32_t category, float value)
        {
            Parameter param;
            param.index = index;
            param.name = name;
            param.category = category;
            param.value = value;
            
            params.push_back(param);
        }

        uint getParamCount(){ return params.size(); };

        Parameter getParam(uint index){ return params.at(index); };

        void setParamValue(uint index, float value){ params.at(index).value = value; };

        String getChannelSelection(){ return channelSelection; };
        float getChannelValue(){ return channelValue; };

        /// @brief Find a parameter in our storage based on name.
        /// @param paramIndex ETC Eos index to find.
        /// @return Returns vector index of parameter if found. Returns -1 otherwise.
        int find(int32_t paramIndex)
        {
            // basic iterator that works in O(N) time which is fine
            for(uint i=0; i < params.size(); i++){
                if(params.at(i).index == paramIndex){
                    return i;
                }
            }
            return -1; // not found
        }


    private:
        std::vector<Parameter> params;

        float channelValue;
        String channelSelection;
};