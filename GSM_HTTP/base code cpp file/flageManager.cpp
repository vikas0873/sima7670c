// flage manager class
#include <map>
#define LSBINDEX 0
#define MSBINDEX 15
#define OPERATIONTYPEANY 1
#define OPERATIONTYPEALL 0

class FlagManager
{
public:
  int getIndexFromMappedInt(std::map<int, lock_unlock_mapped_struct> &dictionary, int status_value)
  {
    for (const auto &pair : dictionary)
    {
      if (pair.second.status == status_value)
      {
        return pair.first;
      }
    }
    return 0; // Value not found
  }

  int setFlagStatus(unsigned int &bit_series, int index, bool status, int type_of_operation = 0)
  {
    if (type_of_operation == OPERATIONTYPEALL)
    {
      if (index >= LSBINDEX && index <= MSBINDEX)
      {
        if (status)
        {
          bit_series |= (1 << index); // Set the bit to 1
        }
        else
        {
          bit_series &= ~(1 << index); // Set the bit to 0
        }
      }
      else
      {
        return -1;
      }
    }
    else if (type_of_operation == OPERATIONTYPEANY)
    {
      if (index >= LSBINDEX && index <= MSBINDEX)
      {
        bit_series = (1 << index); // Set the desired bit to 1, others to 0
      }
      else
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }
    return 1;
  };
  int getFlagStatus(unsigned int &bit_series, int index = 0, int type_of_operation = 0)
  {
    if (type_of_operation == OPERATIONTYPEALL)
    {
      if (index >= LSBINDEX && index <= MSBINDEX)
      {
        return (bit_series >> index) & 1;
      }
      return 0;
    }
    else if (type_of_operation == OPERATIONTYPEANY)
    {
      for (int i = 0; i <= MSBINDEX; ++i)
      {
        if ((bit_series >> i) & 1)
        {
          return i; // this function returns the index of 1 in bitseries
        }
      }
      return 0; // No variable is set to 1
    }
  };
};