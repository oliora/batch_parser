//
//  main.cpp
//  batch_parser
//
//  Created by Andrey Upadyshev on 14.07.13.
//  Copyright (c) 2013 Andrey Upadyshev. All rights reserved.
//

#include "parser2.h"
#include <iostream>
#include <fstream>
#include <vector>

namespace
{
    typedef std::vector<char> Buffer;
    
    Buffer readFile(const char *filename)
    {
        std::ifstream is(filename);
        
        if (!is)
            throw std::runtime_error("Can not open file");
        
        is.unsetf(std::ios::skipws);
        
        is.seekg(0, std::ios_base::end);
        
        Buffer res(is.tellg());
        
        is.seekg(0, std::ios_base::beg);
        const size_t realSize = is.rdbuf()->sgetn(&res.front(), res.size()*2);
        res.resize(realSize);
        
        return res;
    }

}


int main(int argc, const char * argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cout << "No filename specified" << std::endl;
            return 2;
        }
    
        const Buffer buf(readFile(argv[1]));
        
        //std::cout.write(&buf.front(), buf.size());
    
        batch_parser2::parse(buf.begin(), buf.end());
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        return 1;
    }
    
    return 0;
}

