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
            params.clear();
        }

        void setChannel(String ChannelSelection, float ChannelValue)
        {
            channelValue =  ChannelValue;
            channelSelection = ChannelSelection;
        }

        void addParam(Parameter param)
        {
            params.push_back(param);
        }

        uint getParamCount(){ return params.size(); };

        Parameter getParam(uint index){ return params.at(index); };

        void setParamValue(uint index, float value){ params.at(index).value = value; };

        String getChannelSelection(){ return channelSelection; };
        float getChannelValue(){ return channelValue; };

        /// @brief Find a parameter in our storage based on name.
        /// @param paramName Name to find.
        /// @return Returns index of parameter if found. Returns -1 otherwise.
        int find(String paramName)
        {
            // basic iterator that works in O(N) time which is fine
            for(uint i=0; i < params.size(); i++){
                if(params.at(i).name.equals(paramName)){
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