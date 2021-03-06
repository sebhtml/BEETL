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

///A tutorial about suffix arrays.

#include "Alphabet.hh"
#include "Timer.hh"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <queue>

using namespace std;


void getFileName( const string &stem, const char code, const int pile,
                  string &fileName )
{
    fileName = stem;
    fileName += '-';
    fileName += code;
    fileName += '0';
    assert( pile <= 9 );
    fileName += ( char )( 48 + pile );
    //  cerr << "Made file name " << fileName << endl;
}

struct SuffixInfo
{
    //  FILE* pFile_;
    unsigned char fileNum_;
    unsigned char bwtSymbol_;
    const char *pSeq_;
    unsigned loc_;
};

bool operator<( const SuffixInfo &lhs, const SuffixInfo &rhs )
{
    //  assert(1==0);
    int compare( strcmp( lhs.pSeq_ + lhs.loc_, rhs.pSeq_ + rhs.loc_ ) );

    //  if (compare==0) cout << "Hello" << endl;
    // cout << lhs.fileNum_ << " " << lhs.loc_ << " " << rhs.fileNum_ << " " << rhs.loc_ << " " << lhs.pSeq_+lhs.loc_ << " " << (long)rhs.pSeq_ << endl;
    return ( ( compare == 0 ) ? ( lhs.fileNum_ > rhs.fileNum_ ) : ( compare > 0 ) );

    //  return ((compare==0)?(lhs.fileNum_>rhs.fileNum_):(compare>0));

    //         return (strcmp(lhs.pSeq_+lhs.loc_,rhs.pSeq_+rhs.loc_)>0);
}





void readFastaFile( const char *name, vector<char> &data )
{
    FILE *pFile = fopen( name, "r" );
    assert( pFile != NULL );
    fseek( pFile, 0, SEEK_END );
    data.resize( ftell( pFile ) );
    cerr << "Opened file " << name << " of size " << data.size() << endl;
    fseek( pFile, 0, SEEK_SET );

    char *pStartChar( &data[0] );
    char *pThisChar( &data[0] );

    assert( fgets( pThisChar, 1024, pFile ) != NULL );
    assert( *pThisChar == '>' );
    cerr << "Read first line " << pThisChar;

    while ( fgets( pThisChar, 1024, pFile ) != NULL )
    {
        pThisChar += strlen( pThisChar ) - 1;
    }

    fclose( pFile );
    assert( *pThisChar == '\n' );
    *pThisChar = '\0';
    data.resize( pThisChar - pStartChar + 1 );

} // ~readFastaFile


int main ( int numArgs, const char *args[] )
{

    if ( numArgs < 4 )
    {
        cerr << "Usage: " << args[0] << " pileNum outputPrefix files" << endl;
        exit( EXIT_FAILURE );
    }

    int pileNum( atoi( args[1] ) );

    cerr << "Will merge BWTs from pile " << pileNum << endl;

    assert( ( pileNum >= 0 ) && ( pileNum < alphabetSize ) );

    cerr << "Output file will be named " << args[2] << endl;

    vector<vector<char> > chromSeqs;

    //  String<char> fasta_tag;
    vector<FILE *> files, filesBWT;
    FILE *pFileArray;
    FILE *pFileBWT;
    FILE *pFileChrom;

    string fileNameStem( args[2] ), fileName;

    getFileName( fileNameStem, 'A', pileNum, fileName );
    cerr << "Will send suffix array entries to " << fileName << endl;
    pFileArray = fopen( fileName.c_str(), "w" );
    assert( pFileArray != NULL );

    getFileName( fileNameStem, 'B', pileNum, fileName );
    cerr << "Will send BWT entries to " << fileName << endl;
    pFileBWT = fopen( fileName.c_str(), "w" );
    assert( pFileBWT != NULL );

    getFileName( fileNameStem, 'C', pileNum, fileName );
    cerr << "Will send suffix array entries to " << fileName << endl;
    pFileChrom = fopen( fileName.c_str(), "w" );
    assert( pFileChrom != NULL );


    SuffixInfo thisSuffix;
    priority_queue<SuffixInfo, vector<SuffixInfo> > pq;




    for ( int i( 3 ); i < numArgs; i++ )
    {
        chromSeqs.push_back( vector<char>() );
        readFastaFile( args[i], chromSeqs.back() );


        cerr << "Sequence is of length " << chromSeqs.back().size() << endl;

        fileNameStem = "bwt_" + ( string )args[i];
        getFileName( fileNameStem, 'A', pileNum, fileName );
        cerr << "Will open suffix array file " << fileName << endl;
        files.push_back( NULL );
        filesBWT.push_back( NULL );
        files.back() = fopen( fileName.c_str(), "r" );
        if ( files.back() != NULL )
        {
            getFileName( fileNameStem, 'B', pileNum, fileName );
            cerr << "Will open BWT file " << fileName << endl;
            filesBWT.back() = fopen( fileName.c_str(), "r" );
            assert( filesBWT.back() != NULL );
        } // ~if
        else
        {
            cerr << "Warning: could not open " << fileName << endl;
        }
    }

    for ( unsigned int i( 0 ); i < chromSeqs.size(); i++ )
    {
        if ( files[i] != NULL )
        {
            thisSuffix.pSeq_ = &( chromSeqs[i][0] );
            //    cerr << (long)thisSuffix.pSeq_ << endl;

            thisSuffix.fileNum_ = ( unsigned char )i;

            assert( fread( &thisSuffix.loc_,
                           sizeof( unsigned ), 1, files[i] ) == 1 );
            assert( fread( &thisSuffix.bwtSymbol_,
                           sizeof( unsigned char ), 1, filesBWT[i] ) == 1 );

            //    for (int j(0);j<chromSeqs.size();++j) cout << i << " " << (long)&(chromSeqs[j])[0] << endl;
            pq.push( thisSuffix );
        }
    } // ~for

    int j( 0 );

#ifdef DEBUG
    const char *prevSuff( NULL );
    const char *thisSuff( NULL );
    unsigned char prevNum;
#endif

    while ( !pq.empty() )
    {
        thisSuffix = pq.top();

#ifdef DEBUG
        for ( int jj( 0 ); jj < 50; jj++ )
            cout << *( thisSuffix.pSeq_ + thisSuffix.loc_ + jj );
        cout << endl;
        thisSuff = thisSuffix.pSeq_ + thisSuffix.loc_;
        if ( prevSuff != NULL )
        {
            assert( ( strcmp( prevSuff, thisSuff ) < 1 ) ||
                    ( ( strcmp( prevSuff, thisSuff ) == 0 ) && ( prevNum < thisSuffix.fileNum_ ) ) );

        }
        prevSuff = thisSuff;
        prevNum = thisSuffix.fileNum_;
#endif
        pq.pop();
        assert( fwrite( &thisSuffix.loc_, sizeof( unsigned ), 1, pFileArray ) == 1 );
        assert( fwrite( &thisSuffix.bwtSymbol_, sizeof( unsigned char ), 1, pFileBWT ) == 1 );
        assert( fwrite( &thisSuffix.fileNum_, sizeof( unsigned char ), 1, pFileChrom ) == 1 );

        if ( ( j % 1000000 ) == 0 ) cout << "." << endl;
        j++;
#ifdef DEBUG
        for ( int j( 0 ); j < 50; j++ )
            cout << *( thisSuffix.pSeq_ + thisSuffix.loc_ + j );
        cout << endl;
#endif
        if ( fread( &thisSuffix.loc_, sizeof( unsigned ), 1,
                    files[thisSuffix.fileNum_] ) == 1 )
        {
            assert( fread( &thisSuffix.bwtSymbol_, sizeof( unsigned char ), 1,
                           filesBWT[thisSuffix.fileNum_] ) == 1 );
            pq.push( thisSuffix );
        }
    } // ~while


    for ( unsigned int i( 0 ); i < files.size(); i++ )
    {
        if ( files[i] != NULL )
        {
            fclose( files[i] );
        }

        if ( filesBWT[i] != NULL )
        {
            fclose( filesBWT[i] );
        }
    }

    fclose( pFileArray );
    fclose( pFileBWT );
    fclose( pFileChrom );

} // ~main


