/**
 ** Copyright (c) 2011 Illumina, Inc.
 **
 **
 ** This software is covered by the "Illumina Non-Commercial Use Software
 ** and Source Code License Agreement" and any user of this software or
 ** source file is bound by the terms therein (see accompanying file
 ** Illumina_Non-Commercial_Use_Software_and_Source_Code_License_Agreement.pdf)
 **
 ** This file is part of the BEETL software package.
 **
 ** Citation: Markus J. Bauer, Anthony J. Cox and Giovanna Rosone
 ** Lightweight BWT Construction for Very Large String Collections.
 ** Proceedings of CPM 2011, pp.219-231
 **
 **/


/******************************************************************************
* It assumes that:
* -  the length of each sequence is 100.
* -  the alphabet is $ACGNTZ
******************************************************************************/

#include "BWTCollection.hh"

#include "BCRexternalBWT.hh"


namespace SXSI
{
/**
 * Init bwt collection
 *
 * See BCRexternalBWT.h for more details.
 */
BWTCollection *BWTCollection::InitBWTCollection( char *file1, char *fileOut, int mode, CompressionFormatType outputCompression )
{
    BWTCollection *result =
        new BCRexternalBWT( file1, fileOut, mode, outputCompression );
    return result;
}
}
