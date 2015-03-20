#include "Trigger.h"

ClassImp(Trigger)

namespace
{
  const Int_t N_TRIG = 10;
  enum trigger_enum {kAll,kJP2,kJP1,kJP0,kLgBS3,kLgBS2,kLgBS1,kSmBS3,kSmBS2,kSmBS1};

  char * trigger_name(Int_t num)
  {
    switch(num)
    {
      case kAll: return (char*) "all"; break;
      case kJP2: return "JP2"; break;
      case kJP1: return "JP1"; break;
      case kJP0: return "JP0"; break;
      case kLgBS3: return "LgBS3"; break;
      case kLgBS2: return "LgBS2"; break;
      case kLgBS1: return "LgBS1"; break;
      case kSmBS3: return "SmBS3"; break;
      case kSmBS2: return "SmBS2"; break;
      case kSmBS1: return "SmBS1"; break;
    };
    fprintf(stderr,"ERROR: invalid trigger number\n");
    return 0;
  };
};

Trigger::Trigger()
{
  N = N_TRIG;

  // build mask table
  mask_map.insert(std::pair<Int_t,Int_t>(kAll,0xFFE00));

  mask_map.insert(std::pair<Int_t,Int_t>(kJP2,0x10000));
  mask_map.insert(std::pair<Int_t,Int_t>(kJP1,0x20000));
  mask_map.insert(std::pair<Int_t,Int_t>(kJP0,0x40000));

  mask_map.insert(std::pair<Int_t,Int_t>(kSmBS3,0x800));
  mask_map.insert(std::pair<Int_t,Int_t>(kSmBS2,0x400));
  mask_map.insert(std::pair<Int_t,Int_t>(kSmBS1,0x200));

  mask_map.insert(std::pair<Int_t,Int_t>(kLgBS3,0x4000));
  mask_map.insert(std::pair<Int_t,Int_t>(kLgBS2,0x2000));
  mask_map.insert(std::pair<Int_t,Int_t>(kLgBS1,0x1000));
};

Int_t Trigger::Index(char * trigger0)
{
  for(Int_t t=0; t<N_TRIG; t++)
  {
    if(!strcmp(trigger_name(t),trigger0)) return t;
  };
  fprintf(stderr,"ERROR: invalid trigger name in Trigger::Index\n");
  return 0;
};

Int_t Trigger::Mask(char * trigger0)
{
  return mask_map[Index(trigger0)];
};

Int_t Trigger::Mask(Int_t num0)
{
  return mask_map[num0];
};

char * Trigger::Name(Int_t num0)
{
  return trigger_name(num0);
};
