/*=======================================================================

 *
 * filename.h
 *
 * Header defining file names used with TIesr.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *

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
