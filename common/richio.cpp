
/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2007-2010 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2007 Kicad Developers, see change_log.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include <cstdarg>

#include "richio.h"

// This file defines 3 classes useful for working with DSN text files and is named
// "richio" after its author, Richard Hollenbeck, aka Dick Hollenbeck.


//-----<LINE_READER>------------------------------------------------------

LINE_READER::LINE_READER( unsigned aMaxLineLength )
{
    lineNum = 0;

    if( aMaxLineLength == 0 )       // caller is goofed up.
        aMaxLineLength = LINE_READER_LINE_DEFAULT_MAX;

    maxLineLength = aMaxLineLength;

    // start at the INITIAL size, expand as needed up to the MAX size in maxLineLength
    capacity = LINE_READER_LINE_INITIAL_SIZE;

    // but never go above user's aMaxLineLength, and leave space for trailing nul
    if( capacity > aMaxLineLength+1 )
        capacity = aMaxLineLength+1;

    line = new char[capacity];

    line[0] = '\0';
    length  = 0;
}


void LINE_READER::expandCapacity( unsigned newsize )
{
    // length can equal maxLineLength and nothing breaks, there's room for
    // the terminating nul. cannot go over this.
    if( newsize > maxLineLength+1 )
        newsize = maxLineLength+1;

    if( newsize > capacity )
    {
        capacity = newsize;

        // resize the buffer, and copy the original data
        char* bigger = new char[capacity];

        wxASSERT( capacity >= length+1 );

        memcpy( bigger, line, length );
        bigger[length] = 0;

        delete[] line;
        line = bigger;
    }
}


FILE_LINE_READER::FILE_LINE_READER( FILE* aFile, const wxString& aFileName,
                    bool doOwn,
                    unsigned aStartingLineNumber,
                    unsigned aMaxLineLength ) :
    LINE_READER( aMaxLineLength ),
    iOwn( doOwn ),
    fp( aFile )
{
    source  = aFileName;
    lineNum = aStartingLineNumber;
}


FILE_LINE_READER::~FILE_LINE_READER()
{
    if( iOwn && fp )
        fclose( fp );
}


unsigned FILE_LINE_READER::ReadLine() throw( IO_ERROR )
{
    length  = 0;
    line[0] = 0;

    // fgets always put a terminating nul at end of its read.
    while( fgets( line + length, capacity - length, fp ) )
    {
        length += strlen( line + length );

        if( length == maxLineLength )
            THROW_IO_ERROR( _("Line length exceeded") );

        // a normal line breaks here, once through while loop
        if( length+1 < capacity || line[length-1] == '\n' )
            break;

        expandCapacity( capacity * 2 );
    }

    /* if( length ) RHH: this may now refer to a non-existent line but
     * that leads to better error reporting on unexpected end of file.
     */

        ++lineNum;

    return length;
}


STRING_LINE_READER::STRING_LINE_READER( const std::string& aString, const wxString& aSource ) :
    LINE_READER( LINE_READER_LINE_DEFAULT_MAX ),
    lines( aString ),
    ndx( 0 )
{
    // Clipboard text should be nice and _use multiple lines_ so that
    // we can report _line number_ oriented error messages when parsing.
    source = aSource;
}


STRING_LINE_READER::STRING_LINE_READER( const STRING_LINE_READER& aStartingPoint ) :
    LINE_READER( LINE_READER_LINE_DEFAULT_MAX ),
    lines( aStartingPoint.lines ),
    ndx( aStartingPoint.ndx )
{
    // since we are keeping the same "source" name, for error reporting purposes
    // we need to have the same notion of line number and offset.

    source  = aStartingPoint.source;
    lineNum = aStartingPoint.lineNum;
}

unsigned STRING_LINE_READER::ReadLine() throw( IO_ERROR )
{
    size_t  nlOffset = lines.find( '\n', ndx );

    if( nlOffset == std::string::npos )
        length = lines.length() - ndx;
    else
        length = nlOffset - ndx + 1;     // include the newline, so +1

    if( length )
    {
        if( length >= maxLineLength )
            THROW_IO_ERROR( _("Line length exceeded") );

        if( length+1 > capacity )   // +1 for terminating nul
            expandCapacity( length+1 );

        wxASSERT( ndx + length <= lines.length() );

        memcpy( line, &lines[ndx], length );

        ndx += length;
    }

    ++lineNum;      // this gets incremented even if no bytes were read

    line[length] = 0;

    return length;
}


//-----<OUTPUTFORMATTER>----------------------------------------------------

// factor out a common GetQuoteChar

const char* OUTPUTFORMATTER::GetQuoteChar( const char* wrapee, const char* quote_char )
{
    // Include '#' so a symbol is not confused with a comment.  We intend
    // to wrap any symbol starting with a '#'.
    // Our LEXER class handles comments, and comments appear to be an extension
    // to the SPECCTRA DSN specification.
    if( *wrapee == '#' )
        return quote_char;

    if( strlen(wrapee)==0 )
        return quote_char;

    bool isFirst = true;

    for(  ; *wrapee;  ++wrapee, isFirst=false )
    {
        static const char quoteThese[] = "\t ()"
            "%"     // per Alfons of freerouting.net, he does not like this unquoted as of 1-Feb-2008
            "{}"    // guessing that these are problems too
            ;

        // if the string to be wrapped (wrapee) has a delimiter in it,
        // return the quote_char so caller wraps the wrapee.
        if( strchr( quoteThese, *wrapee ) )
            return quote_char;

        if( !isFirst  &&  '-' == *wrapee )
            return quote_char;
    }

    return "";  // caller does not need to wrap, can use an unwrapped string.
}


int OUTPUTFORMATTER::vprint( const char* fmt,  va_list ap )  throw( IO_ERROR )
{
    int ret = vsnprintf( &buffer[0], buffer.size(), fmt, ap );
    if( ret >= (int) buffer.size() )
    {
        buffer.reserve( ret+200 );
        ret = vsnprintf( &buffer[0], buffer.size(), fmt, ap );
    }

    if( ret > 0 )
        write( &buffer[0], ret );

    return ret;
}


int OUTPUTFORMATTER::sprint( const char* fmt, ... )  throw( IO_ERROR )
{
    va_list     args;

    va_start( args, fmt );
    int ret = vprint( fmt, args);
    va_end( args );

    return ret;
}


int OUTPUTFORMATTER::Print( int nestLevel, const char* fmt, ... ) throw( IO_ERROR )
{
#define NESTWIDTH           2   ///< how many spaces per nestLevel

    va_list     args;

    va_start( args, fmt );

    int result = 0;
    int total  = 0;

    for( int i=0; i<nestLevel;  ++i )
    {
        // no error checking needed, an exception indicates an error.
        result = sprint( "%*c", NESTWIDTH, ' ' );

        total += result;
    }

    // no error checking needed, an exception indicates an error.
    result = vprint( fmt, args );

    va_end( args );

    total += result;
    return total;
}


std::string OUTPUTFORMATTER::Quoted( const std::string& aWrapee ) throw( IO_ERROR )
{
    // derived class's notion of what a quote character is
    char quote          = *GetQuoteChar( "(" );

    // Will the string be wrapped based on its interior content?
    const char* squote  = GetQuoteChar( aWrapee.c_str() );

    std::string wrapee  = aWrapee;  // return this

    // Search the interior of the string for 'quote' chars
    // and replace them as found with duplicated quotes.
    // Note that necessarily any string which has internal quotes will
    // also be wrapped in quotes later in this function.
    for( unsigned i=0;  i<wrapee.size();  ++i )
    {
        if( wrapee[i] == quote )
        {
            wrapee.insert( wrapee.begin()+i, quote );
            ++i;
        }
        else if( wrapee[i]=='\r' || wrapee[i]=='\n' )
        {
            // In a desire to maintain accurate line number reporting within DSNLEXER
            // a decision was made to make all S-expression strings be on a single
            // line.  You can embed \n (human readable) in the text but not
            // '\n' which is 0x0a.
            THROW_IO_ERROR( _( "S-expression string has newline" ) );
        }
    }

    if( *squote )
    {
        // wrap the beginning and end of the string in a quote.
        wrapee.insert( wrapee.begin(), quote );
        wrapee.insert( wrapee.end(), quote );
    }

    return wrapee;
}


//-----<STRING_FORMATTER>----------------------------------------------------

void STRING_FORMATTER::write( const char* aOutBuf, int aCount ) throw( IO_ERROR )
{
    mystring.append( aOutBuf, aCount );
}

void STRING_FORMATTER::StripUseless()
{
    std::string  copy = mystring;

    mystring.clear();

    for( std::string::iterator i=copy.begin();  i!=copy.end();  ++i )
    {
        if( !isspace( *i ) && *i!=')' && *i!='(' && *i!='"' )
        {
            mystring += *i;
        }
    }
}


//-----<STREAM_OUTPUTFORMATTER>--------------------------------------

const char* STREAM_OUTPUTFORMATTER::GetQuoteChar( const char* wrapee )
{
    return OUTPUTFORMATTER::GetQuoteChar( wrapee, quoteChar );
}


void STREAM_OUTPUTFORMATTER::write( const char* aOutBuf, int aCount ) throw( IO_ERROR )
{
    int lastWrite;

    // This might delay awhile if you were writing to say a socket, but for
    // a file it should only go through the loop once.
    for( int total = 0;  total<aCount;  total += lastWrite )
    {
        lastWrite = os.Write( aOutBuf, aCount ).LastWrite();

        if( !os.IsOk() )
        {
            THROW_IO_ERROR( _( "OUTPUTSTREAM_OUTPUTFORMATTER write error" ) );
        }
    }
}

