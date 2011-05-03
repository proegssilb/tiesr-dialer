/*=======================================================================
 filename.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED

======================================================================*/

#ifndef _FILENAME_H
#define _FILENAME_H

static const char * const file_names[] =  {
   "word.lis",
   "net.bin",
   "hmm.bin",
   "mu.bin",
   "var.bin",
   "tran.bin",
   "pdf.bin",
   "mixture.bin",
   "gconst.bin",
   "dim.bin",

   /* scaling factor for mean and/or variance if they are in byte format */
   "scale.bin", 
   /* file defining configuration of the model file information */
   "config.bin",
   /* file defining mapping from hmm to monophone or other model index */
   "hmm2phone.bin"
};

#endif
