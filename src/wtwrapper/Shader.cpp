

#include "Shader.h"

#include <vector>
#include <string>
#include <sstream>


namespace
{
    std::map<std::string, GLenum> initTypes()
    {
        // http://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetActiveUniform.xml
        std::map<std::string, GLenum> uniformAttributeTypes;
        
        /* taken from Uniform Types in gles2/gl2.h*/
        uniformAttributeTypes["bool"] = GL_BOOL;
        uniformAttributeTypes["int"] = GL_INT;
        uniformAttributeTypes["float"] = GL_FLOAT;
        
        uniformAttributeTypes["vec2"] = GL_FLOAT_VEC2;
        uniformAttributeTypes["vec3"] = GL_FLOAT_VEC3;
        uniformAttributeTypes["vec4"] = GL_FLOAT_VEC4;

        uniformAttributeTypes["ivec2"] = GL_INT_VEC2;
        uniformAttributeTypes["ivec3"] = GL_INT_VEC3;       
        uniformAttributeTypes["ivec4"] = GL_INT_VEC4;      

        uniformAttributeTypes["bvec2"] = GL_BOOL_VEC2;
        uniformAttributeTypes["bvec3"] = GL_BOOL_VEC3;
        uniformAttributeTypes["bvec4"] = GL_BOOL_VEC4;

        uniformAttributeTypes["mat2"] = GL_FLOAT_MAT2;
        uniformAttributeTypes["mat3"] = GL_FLOAT_MAT3;
        uniformAttributeTypes["mat4"] = GL_FLOAT_MAT4;

        uniformAttributeTypes["sampler2D"] =GL_SAMPLER_2D;
        uniformAttributeTypes["samplerCube"] =GL_SAMPLER_CUBE;

        return uniformAttributeTypes;
    }

    // mapping: uniform type name -> size, type
    static std::map<std::string, GLenum> s_uniformAttributeTypes
        = initTypes();


}

namespace WTW
{

    // extract a chunk to the closest given character. Returns the found character and chunk
    // and leaves the stream with the chunk extracted
    char seekNextOfCharacters(std::stringstream &mystream, std::string & foundChunk, const std::string & characters)
    {
        std::stringstream::pos_type startPos = mystream.tellg();
        
        unsigned int numChar = 0;
        while (mystream.good())     // loop while extraction from file is possible
        {
            numChar++;
            const char c = mystream.get();       // get character from file
            
            // check if one of the limiting characters was found
            size_t firstChar =  characters.find_first_of(c); 
            
            if (firstChar != std::string::npos)
            {
                // found character. extract chunk from string and return found char
                mystream.seekg(startPos);
                
                char chunk [1024];
                if (numChar>1024) std::cout << "Warning: extracting a chunk too big from the shader"<< std::endl;
                mystream.get(chunk, numChar);
                mystream.get(); // advance one more to skip the delimiter
                foundChunk = chunk;
                return characters[firstChar];
            }
        }
        foundChunk = "";
        return 0;
    }


void Shader::parseShaderSource(std::string & shaderSource)
{
    // reset arrays
    m_uniforms.clear();
    m_attributes.clear();

    std::string line;
    std::stringstream mystream (shaderSource);
    
    // structs seen
    std::map<std::string, std::vector<Uniform> > uniformStructs;
    //unsigned int withinStruct = 0;
    std::string currentStruct ="";

    // parse shaderSource
    while ( mystream.good() )
    {
        // split by lines
        std::getline (mystream,line);

        //cout << line << endl;

        // handle line comments        
        size_t firstChar =  line.find_first_of("//");   
        if (firstChar != std::string::npos) 
        {
            // remove everything behind //
            line.erase(firstChar);
        }

        // TODO: handle multi line comments
        

        // start another loop to split statements ;
        // split by ;

        std::stringstream lineStream (line);

       
        while(lineStream.good())
        //while(std::getline (lineStream,line, ';'))
        {
            // read next chunk from the linestream
            char splitChar = seekNextOfCharacters(lineStream, line, ";{}");

            // statementStream contains everything until the next ;
            std::stringstream statementStream(line);

            std::string firstWord;
            statementStream >> firstWord;

            // handle struct definition
            if (currentStruct != "")
            {
                // TODO: handle nested scopes {}

                if ( splitChar == 0)
                {
                    continue;
                }
                else if ( splitChar == '{')
                {
                    // will read next chunk, if in the same line
                    continue; 
                }else if (splitChar == '}')
                {
                    // will read next chunk, if in the same line
                    // finished struct
                    currentStruct = "";
                    continue; 
                }
                else
                {
                    // read struct member
                    std::string dataType, name;
                    dataType = firstWord; // data type already has been read
                    statementStream >> name;

                    uniformStructs[currentStruct].push_back(Uniform(name, s_uniformAttributeTypes[dataType], 1));
                }
            }else if (firstWord == "struct" )
            {
                std::string name;
                statementStream >> name;
                currentStruct = name;

                currentStruct = name;
            }
            // check for uniforms and attributes
            else if (firstWord == "uniform" || firstWord == "attribute")
            {
                std::string dataType, name;
                statementStream >> dataType;
                statementStream >> name;
                //statementStream >> arrayBrackets;

                // check data type
                if (s_uniformAttributeTypes.count(dataType)==0 && uniformStructs.count(dataType)==0)
                {
                    std::cout << "Shader: Unknown datatype " << dataType << std::endl;
                    return;
                }
                if (firstWord == "uniform")
                {
                    // take care of arrays of uniforms [] for the size
                    // ...and arrays of structs
                    // see http://www.opengl.org/wiki/GLSL_Uniform

                    // check if it is an array (not considering multi dim arrays [2][2])
                    unsigned int arrayElements = 1;
                    size_t firstBracketChar =  name.find_first_of("[");
                    if (firstBracketChar != std::string::npos)
                    {
                        size_t lastBracketChar = name.find_first_of("]", firstBracketChar);
                        assert(lastBracketChar != std::string::npos); // mal formed glsl : [ without ]
                        const std::string numElementsStr = name.substr(firstBracketChar+1, lastBracketChar-1-firstBracketChar);
                        arrayElements = boost::lexical_cast<unsigned int>(numElementsStr);
                        
                        // strip the [] from the name
                        name.erase(firstBracketChar, lastBracketChar+1-firstBracketChar);
                    }


                    // a elementary datatype
                    if (s_uniformAttributeTypes.count(dataType)>0)
                    {
                        m_uniforms.push_back(Uniform(name,s_uniformAttributeTypes[dataType], arrayElements));
                    }else if (uniformStructs.count(dataType)>0)
                    {
                        // for each struct member, we have to store it as a separate Uniform
                        // <struct instance name>.<struct member name>
                        
                        // for arrays of structs, each array element and struct element has its own location
                        // <struct instance name>[element index].<struct member name>

                        std::string arrayName = name;
                        for (size_t j = 0, jEnd = arrayElements; j<jEnd; j++)
                        {
                            // only add the [] if there is more then one element
                            if (arrayElements !=1) arrayName += "[" + boost::lexical_cast<std::string>(j) + "]";
                            for (size_t i = 0, iEnd = uniformStructs[dataType].size(); i<iEnd; i++)
                            {
                                std::string memberName = arrayName + "." + uniformStructs[dataType][i].m_name;
                                m_uniforms.push_back(Uniform(memberName,uniformStructs[dataType][i].m_type, 1));
                            }
                        }
                    }else
                    {
                        // must not happen
                        assert(false);
                    }


                }else if (firstWord == "attribute")
                {
                    // TODO: take care of arrays of attributes [] (?) for the size
                    // TODO: handle arrays with #define'ed sizes
                    m_attributes.push_back(Attribute(name, s_uniformAttributeTypes[dataType], 1));
                }
            }
        }
        



        //// check if a struct definition is finished
        //if (withinStruct > 0 )
        //{
        //    // TODO: handle nested scopes {}
        //    firstChar =  line.find_first_of("{");   
        //    if (firstChar != std::string::npos) 
        //    {
        //        withinStruct++;
        //    }

        //    firstChar =  line.find_first_of("}");   
        //    if (firstChar != std::string::npos) 
        //    {
        //        withinStruct--;
        //        if (withinStruct == 0) currentStruct = "";
        //    }
        //}
    }
}
        
}
