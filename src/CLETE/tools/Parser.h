/* Parser.h
 *
 * Copyright (c) 2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3 (LGPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This is a derivative work based on PETE-2.1.0, produced in compliance
 * with the original ACL license reproduced below, and which should not be
 * confused with the original software.
 */

// ACL:license
// ----------------------------------------------------------------------
// This software and ancillary information (herein called "SOFTWARE")
// called PETE (Portable Expression Template Engine) is
// made available under the terms described here.  The SOFTWARE has been
// approved for release with associated LA-CC Number LA-CC-99-5.
// 
// Unless otherwise indicated, this SOFTWARE has been authored by an
// employee or employees of the University of California, operator of the
// Los Alamos National Laboratory under Contract No.  W-7405-ENG-36 with
// the U.S. Department of Energy.  The U.S. Government has rights to use,
// reproduce, and distribute this SOFTWARE. The public may copy, distribute,
// prepare derivative works and publicly display this SOFTWARE without 
// charge, provided that this Notice and any statement of authorship are 
// reproduced on all copies.  Neither the Government nor the University 
// makes any warranty, express or implied, or assumes any liability or 
// responsibility for the use of this SOFTWARE.
// 
// If SOFTWARE is modified to produce derivative works, such modified
// SOFTWARE should be clearly marked, so as not to confuse it with the
// version available from LANL.
// 
// For more information about PETE, send e-mail to pete@acl.lanl.gov,
// or visit the PETE web page at http://www.acl.lanl.gov/pete/.
// ----------------------------------------------------------------------
// ACL:license
// ----------------------------------------------------------------------
// $RCSfile: Parser.h,v $   $Author: sa_smith $
// $Revision: 1.10 $   $Date: 2000/08/03 00:18:02 $
// ----------------------------------------------------------------------

/* DAR */

#ifndef _PARSER_H
#define _PARSER_H

#include <iostream>

using std::ostream;
using std::istream;
using std::getline;
using std::ios;
using std::cerr;
using std::endl;

#include <map>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::map;

//-----------------------------------------------------------------------------
//
// DESCRIPTION
//    The Token types for our little parser:
//      o pre-defined "keyword" tokens
//      o a group name token
//      o a separator token
//      o an equals sign token
//      o an end-o-line token
//      o an end-o-file token
//      o no token
//
//-----------------------------------------------------------------------------

typedef int TokenType;

const int KEY1 = 0;
const int KEY2 = 1;
const int KEY3 = 2;
const int KEY4 = 3;
const int KEY5 = 4;

const int GROUP = 100;
const int SEP = 101;
const int EQUALS = 102;
const int STRING = 103;
const int EOL = 104;
const int EOFile = 105;
const int PREFIX = 106;
const int SUFFIX = 107;
const int TEXTTOKEN = 108;
const int NOTOKEN = 999;

//-----------------------------------------------------------------------------
//
// CLASS NAME
//    Token
//
// DESCRIPTION
//    The Tokens returned by our parser. Contains fields for getting the
//    text, type, and line number associated with the token.
//
//-----------------------------------------------------------------------------

class Token {
public:

  //---------------------------------------------------------------------------
  // Constructors and initialization functions.

  Token(const string &str)
  : str_m(str), pos_m(0), len_m(0), type_m(NOTOKEN), line_m(0)
  { }
  
  Token(const string &str, int pos, int len, TokenType type, int line)
  : str_m(str), pos_m(pos), len_m(len), type_m(type), line_m(line)
  { }

  void set(int pos, int len,  TokenType type, int line)
  {
    pos_m = pos;
    len_m = len;
    type_m = type;
    line_m = line;
  }
  
  //---------------------------------------------------------------------------
  // Return information about the token.

  string text() const
  {
    return str_m.substr(pos_m, len_m);
  }
  
  TokenType type() const
  {
    return type_m;
  }
  
  int line() const
  {
    return line_m;
  }

  int isKeyword() const
  {
    return type_m >= 0 && type_m < 100;
  }

private:

  const string &str_m;
  int pos_m, len_m, line_m;
  TokenType type_m;
};

inline ostream &operator<<(ostream &os, const Token &tok)
{
  os << static_cast<int>(tok.type()) << ": " << tok.text();
  
  return os;
}


//-----------------------------------------------------------------------------
//
// CLASS NAME
//    Lexer
//
// DESCRIPTION
//    A tiny lexical analyzer that turns text from an istream into tokens.
//
//-----------------------------------------------------------------------------

class Lexer {
public:

  //---------------------------------------------------------------------------
  // Constructor. Initializes the lexer and reads text into a string for future
  // processing.

  Lexer(istream &is, const string &fn)
  : file_m(fn), tok_m(str_m)
  {
    line_m = 1;
    pos_m = 0;

    // Read input file.
        
    string buffer;  
    while (getline(is, buffer) || is.gcount())
      {
        if (is.eof())
          {
            str_m += buffer;
          }
        else if (is.fail())
          {
            str_m += buffer;
            is.clear(is.rdstate() & ~ios::failbit);
          }
        else
          {
            str_m += buffer;
            str_m += '\n';
          }
      }
  }
  
  //---------------------------------------------------------------------------
  // Adds a new keyword to the Lexical Analyzer. Up to 100 are supported.

  TokenType addKeyword(const string &kw)
  {
    TokenType kwNum = kws_m.size();
    
    if (kwNum > 100)
      {
	cerr << "ERROR: too many keywords." << endl;
	exit(25);
      }

    kws_m.push_back(kw);
    return kwNum;
  }

  //---------------------------------------------------------------------------
  // Returns the next token.

  const Token &nextToken()
  {
    while (pos_m < str_m.size() && 
      (str_m[pos_m] == ' ' || str_m[pos_m] == '\t'))
        pos_m++;
        
    if (pos_m == str_m.size())
      {
        tok_m.set(pos_m, 0, EOFile, line_m);
      }
    else
      {
        char c = str_m[pos_m];
        if (c == '\n')
          {
            tok_m.set(pos_m++, 1, EOL, line_m++);
          }
        else if (c == '-')
          {
            int pos = pos_m;
            do {
              pos_m++;
            } while (pos_m < str_m.size() && str_m[pos_m] == '-');
            tok_m.set(pos, pos_m - pos, SEP, line_m);
          }
        else if (c == '=')
          {
            tok_m.set(pos_m++, 1, EQUALS, line_m);
          }
//        else if (c == '\"')
        else if (c == '@') /* XXX using quotes is not good, use @ instead -DAR */
          {
            int pos = pos_m;
            do {
              pos_m++;
            } while (pos_m < str_m.size() && 
//                     str_m[pos_m] != '\"' /* && str_m[pos_m] != '\n' */);
                     str_m[pos_m] != '@' /* && str_m[pos_m] != '\n' */);
            if (pos_m == str_m.size() /* || str_m[pos_m] == '\n' */)
              {
                cerr << "ERROR: untermininated string.\n"
                  << "File: \"" << file_m << "\"; line: " << line_m
                  << endl;
                exit(1);
              }
            else
              {
                tok_m.set(pos + 1, ++pos_m - pos - 2, STRING, line_m);
              }
          }
//        else if (isalpha(c))
        else if (isalpha(c) && c != '@')
          {
	    // Find the extent of the alphanumeric token.

            int pos = pos_m;
            do {
              pos_m++;
            } while (pos_m < str_m.size() && isalpha(str_m[pos_m]));

	    // Check for canned text tokens first.

	    if (str_m.substr(pos, pos_m - pos) == "prefix")
	      tok_m.set(pos, pos_m - pos, PREFIX, line_m);
	    else if (str_m.substr(pos, pos_m - pos) == "suffix")
	      tok_m.set(pos, pos_m - pos, SUFFIX, line_m);
	    else if (str_m.substr(pos, pos_m - pos) == "TEXT")
	      tok_m.set(pos, pos_m - pos, TEXTTOKEN, line_m);
	    else
	      {
		// Search for a keyword.

		bool foundKeyword = false;
		for (int i = 0; i < kws_m.size(); i++)
		  {
		    if (str_m.substr(pos, pos_m - pos) == kws_m[i])
		      {
			tok_m.set(pos, pos_m - pos, i, line_m);
			foundKeyword = true;
			break;
		      }
		  }
		
		// If it isn't a keyword, it is a group name.

		if (!foundKeyword)
		  {
		    tok_m.set(pos, pos_m - pos, GROUP, line_m);
		  }
	      }
          }
        else
          {
            cerr << "ERROR: unrecognizable input found.\n"
              << "File: \"" << file_m << "\"; line: " << line_m
              << endl;
            exit(2);
          }
      }

    return tok_m;
  }            

  //---------------------------------------------------------------------------
  // Returns the current token.

  const Token &currentToken()
  {
    return tok_m;
  }
  
  //---------------------------------------------------------------------------
  // Matches the current token or else generates an error.

  void matchCurrent(TokenType type)
  {
    if (tok_m.type() != type)
      {
        cerr << "ERROR: syntax error.\n"
          << "File: \"" << file_m << "\"; line: " << tok_m.line()
          << endl;
        exit(3);
      }
  }
  
  //---------------------------------------------------------------------------
  // Matches the next token or else generates an error.

  void matchNext(TokenType type)
  {
    nextToken();
    matchCurrent(type);
  }
    
private:
  
  string file_m, str_m;
  vector<string> kws_m;
  Token tok_m;
  int pos_m, line_m;
};
            

//-----------------------------------------------------------------------------
//
// CLASS NAME
//    Parser
//
// DESCRIPTION
//    A tiny parser that reads our input file and stores data for each group
//    of descriptors into a map.
//
//-----------------------------------------------------------------------------

template<class Descriptor>
class Parser {
public:

  //---------------------------------------------------------------------------
  // Constructor.

  Parser(istream &is, const string &fn,
    map<string, vector<Descriptor> > &glist)
  : lexer_m(is, fn), glist_m(glist)
  { }
  
  //--------------------------------------------------------------------------
  // Adds a new keyword to the Lexical Analyzer. Up to 100 are supported.

  TokenType addKeyword(const string &kw)
  {
    return lexer_m.addKeyword(kw);
  }

  //--------------------------------------------------------------------------
  // returns prefix and suffix strings.

  const string &prefixText() const
  {
    return prefixText_m;
  }
  const string &suffixText() const
  {
    return suffixText_m;
  }

  //---------------------------------------------------------------------------
  // Recursive descent parse functions. Stores information into glist_m.
  // (except for prefix and suffix information).

  void parse()
  {
    lexer_m.nextToken();
    while (lexer_m.currentToken().type() != EOFile)
      if (lexer_m.currentToken().type() == EOL)
        lexer_m.nextToken();
      else if (lexer_m.currentToken().type() == PREFIX ||
	       lexer_m.currentToken().type() == SUFFIX)
	prefixSuffix(lexer_m.currentToken().type());
      else
        group();
  }

private:
  
  void prefixSuffix(TokenType t)
  {
    lexer_m.matchCurrent(t);
    lexer_m.matchNext(EOL);
    lexer_m.matchNext(SEP);
    lexer_m.matchNext(EOL);
    lexer_m.matchNext(TEXTTOKEN);
    lexer_m.matchNext(EQUALS);
    lexer_m.matchNext(STRING);

    if (t == PREFIX)
      prefixText_m = lexer_m.currentToken().text();
    else
      suffixText_m = lexer_m.currentToken().text();
  
    lexer_m.matchNext(EOL);
  }

  void group()
  {
    lexer_m.matchCurrent(GROUP);
    group_m = lexer_m.currentToken().text();
    lexer_m.matchNext(EOL);

    lexer_m.nextToken();    
    dataList();
  }
  
  void dataList()
  {
    while (lexer_m.currentToken().type() == SEP)
      {
        lexer_m.matchNext(EOL);
	Descriptor desc;
        while (true)
          {
            const Token &tok = lexer_m.nextToken();
	    if (tok.isKeyword())
              {
		TokenType kw = tok.type();
		lexer_m.matchNext(EQUALS);
		lexer_m.matchNext(STRING);
		desc.addData(kw, lexer_m.currentToken().text());
		lexer_m.matchNext(EOL);
              }
            else
              break;
          }

        glist_m[group_m].push_back(desc);
      }
  }
  
  Lexer lexer_m;
  string group_m, prefixText_m, suffixText_m;
  map<string, vector<Descriptor> > &glist_m;
  
};
  
#endif // _PARSER_H

