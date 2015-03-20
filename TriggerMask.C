// returns trigger mask for given trigger using Akio's L2sum variable

Int_t TriggerMask(const char * trigger)
{
  if(!strcmp(trigger,"all")) return 0xFFE00;

  if(!strcmp(trigger,"JP2")) return 0x10000;
  if(!strcmp(trigger,"JP1")) return 0x20000;
  if(!strcmp(trigger,"JP0")) return 0x40000;

  if(!strcmp(trigger,"SmBS3")) return 0x800;
  if(!strcmp(trigger,"SmBS2")) return 0x400;
  if(!strcmp(trigger,"SmBS1")) return 0x200;

  if(!strcmp(trigger,"LgBS3")) return 0x4000;
  if(!strcmp(trigger,"LgBS2")) return 0x2000;
  if(!strcmp(trigger,"LgBS1")) return 0x1000;
  
  printf("ERROR: invalid trigger\n");
  return 0;
};
