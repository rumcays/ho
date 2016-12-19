/*
MIT License

Copyright (c) 2016 Maciej Kalinski https://github.com/rumcays

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
    @file  ho_sax.hpp

    @description
    This is a part of header-only (see: wiki Header-only) tiny utils library.

    SAX parser with limitations, suitable for parsing small and simple
    XML documents, like configuration files.
    The parser invokes visitor's callbacks with raw begin/end pointers
    to the matched input document's content; these data, except element name,
    may be digested by auxiliary conversion methods, to remove
    extra white spaces, and convert escape sequences.
    The design assumption is that the parser should
    operate on input data wherever possible without extra conversions,
    while the post-processing, including conversions should be done
    with these helper methods, or by the client itself.

    Limitations and features (a short list):
    * not supported: UNICODE
    * skipped: comments, XML Declaration, Processing Instructions (PIs)
    * DTD: only simple cases are parsed, and the whole content is skipped
    * by default no conversion of: attribute values, XML Text, CDATA,
      escape codes
    * auxiliary conversion methods, including escape codes when appropriate:
      - attribute's value: all sequences of whitespaces are converted
        to single space
      - sequences of whitespaces inside the actual text are converted
        to single space; outside of the actual text are removed
    * relax comment usage and content: may occur before XML Declaration;
      allow double hyphens
    * error handling during parsing: parser calls visitors 'error()' method,
      and returns false
    * exceptions: the parser's methods do not throw; all exceptions thrown
      by during parsing by other libraries might be absorbed and handled as
      parse errors, if the HO_SAX_CATCH_EXCEPTIONS macro
      has been defined; note: the macro will become undefined in the end
      of the file

    Usage: see ho_sax_ult.hpp to figure out what should work
    and how to use the parser.

    Requirements: compiler with limited c++11 support (regex, lambdas),
    like VS2010 Express, gcc/clang.

    Caveats and TODOs:
    * it seems that VS2010 Express has a bug, such that following tag
      cannot be matched (and it does not encounter on vs2015 and gcc/clang):
      <title  xml:lang = \"de-DE\"  available = \"true\" >
    * the following regex that I planned to use in toString() method
      does not match multiline patterns; why?
      "^(\\s+)?(.*\\S)?(\\s+)?"
    * On VS2010 Express the following regex that I planned to use
      in toString() method reveals a behavior that could be a bug -
      - for " " input sometimes regex match 3rd group
      "(\\s+)?(\\S+(?:\\s+\\S+)*)?(\\s+)?"
*/

#ifndef HO_SAX_HPP_
#define HO_SAX_HPP_

#include <assert.h>
#include <cstring>
#include <string>
#include <vector>
#include <regex>
#include <utility>

namespace headeronly
{
class XmlSax
{
public: // types
    /// [begin, end) position of a content: element name,
    /// attribute name/value, text, CDATA content
    typedef std::pair<const char*, const char*> String;

    /// Callback base
    /// If an overridden callback function returns false, the parser
    /// stops processing and quits returning false.
    struct Visitor
    {
        virtual ~Visitor() {}

        /// Visiting callbacks. For a node, enter() and exit()
        /// are called once, remained may be called zero or more times.
        /// isEmptyElementTag means e.g. <section/>
        virtual bool enter(
            const String& /*element*/,
            bool /*isEmptyElementTag*/)
        { return true; }
        virtual bool exit(
            const String& /*element*/,
            bool /*isEmptyElementTag*/)
        { return true; }
        virtual bool attribute(
            const String& /*name*/,
            const String& /*value*/)
        { return true; }
        virtual bool text(const String& /*content*/)
        { return true; }
        virtual bool cdata(const String& /*content*/)
        { return true; }

        /// Handle parsing error
        /// @info  error description
        /// @pos  position of the unparsed remainder
        virtual void error(const char* /*info*/, const char* /*docPos*/)
        {}

        /// If true, the parser should perform some validation
        /// requiring extra - potentially expensive - processing.
        virtual bool validate()
        { return false; }
    };

public: // constructors
    XmlSax(Visitor& visitor):
        m_visitor(visitor)
    {}

public: // function members
    // Parsing method; doc refers to a C-style string representing xml document
    // Return false if parsing process failed, or a callback function
    // returned false.
    bool parse(const char* doc)
    {
        assert(doc);

        bool retCode = true;

        // Pointer to unparsed remainder.
        const char* docPos = doc;
#ifdef HO_SAX_CATCH_EXCEPTIONS
        try
#endif // HO_SAX_CATCH_EXCEPTIONS
        {
            static const std::string value =
                "(?:[^<\"]|(?:&(?:lt|gt|amp|apos|quot);))*";
            // Including optional namespace prefix
            static const std::string elementName =
                "(?:" + getReName() + ":)?" + getReName();
            // Including optional "xml:" prefix for special attributes
            static const std::string attributeName =
                "(?:xml:|xmlns:)?" + getReName();
            // One attribute in the list. Preceded by one or more white spaces!
            static const std::string attribute =
                "\\s+(" + attributeName + ")\\s*=\\s*\"(" + value + ")\"";

            // At least the 'version' attribute is required
            static const std::regex regexXmlDeclaration("^<\\?xml(?:\\s+" +
                getReName() + "\\s*=\\s*\"" + value + "\")+\\s*\\?>");
            // https://en.wikipedia.org/wiki/Processing_Instruction
            static const std::regex regexXmlPI("^<\\?(?:" + getReName() +
                ")(?:\\s+" + getReName() + "\\s*=\\s*\"" +
                value + "\")*\\s*\\?>");
            static const std::regex regexXmlCDATA(
                "^<!\\[CDATA\\[((?:[^\\]]|\\](?!\\]>))*)\\]\\]>");

            // Non-empty element rules: no spaces are allowed: "< id"
            static const std::regex regexNodeOpen(
                "^<(" + elementName + ")(?:" + attribute + ")*\\s*(/)?>");
            // Closing element rules: no spaces are allowed: "< /id", "</ id"
            static const std::regex regexNodeClose(
                "^</(" + elementName + ")\\s*>");
            static const std::regex regexNodeAttrList("^" + attribute);

            docPos = skipSpacesAndComments(docPos);
            assert(docPos);

            {
                std::cmatch match;
                if (std::regex_search(docPos, match, regexXmlDeclaration,
                        std::regex_constants::match_continuous))
                {
                    docPos = skipSpacesAndComments(match.suffix().first);
                    assert(docPos);
                }
            }

            docPos = skipDoctype(docPos);
            assert(docPos);

            if (!*docPos)
            {
                // No XML statements, only some spaces, comments and doctype
                assert(docPos == (doc + strlen(doc)));
                return true;
            }

            m_nodeStack.clear();
            do
            {
                assert(retCode);

                const char* tmpPos = nullptr;
                std::cmatch match;
                if(std::regex_search(docPos, match, regexNodeOpen,
                    std::regex_constants::match_continuous))
                {
                    assert(!match.empty() && match.size() > 2);

                    const auto lastMatch = match[match.size() - 1];
                    const bool isEmptyElementTag = lastMatch.matched;

                    assert(!isEmptyElementTag || *lastMatch.first == '/');

                    m_nodeStack.push_back(match[1]);
                    retCode = m_visitor.enter(m_nodeStack.back(), isEmptyElementTag);

                    std::vector<String> attributeNames;
                    std::cmatch attrMatch;
                    for(auto cbegin = match[1].second;
                        retCode &&
                        std::regex_search(
                            cbegin,
                            match.suffix().first,
                            attrMatch,
                            regexNodeAttrList,
                            std::regex_constants::match_continuous);)
                    {
                        assert(attrMatch.size() == 3);

                        m_visitor.attribute(attrMatch[1], attrMatch[2]);

                        if (m_visitor.validate())
                        {
                            const auto it = std::find_if(
                                attributeNames.begin(),
                                attributeNames.end(),
                                [&](const String& v){
                                    return equalStrings(attrMatch[1], v);}
                              );
                            if (it != attributeNames.end())
                            {
                                m_visitor.error(
                                    ("ERROR: duplicated attribute: \"" + 
                                    toStringName(*it) + "\"").c_str(), docPos);
                                retCode = false;
                            }

                            attributeNames.push_back(attrMatch[1]);
                        }

                        cbegin = attrMatch.suffix().first;
                    }

                    if (retCode && isEmptyElementTag)
                    {
                        m_visitor.exit(m_nodeStack.back(), true);
                        m_nodeStack.pop_back();
                    }
                }
                else if(std::regex_search(docPos, match, regexNodeClose,
                    std::regex_constants::match_continuous))
                {
                    assert(!match.empty());

                    if(m_nodeStack.empty())
                    {
                        m_visitor.error(
                            "ERROR: no matching opening attribute statement",
                            docPos);
                        retCode = false;
                    }
                    else
                    {
                        if(!equalStrings(m_nodeStack.back(), match[1]))
                        {
                            m_visitor.error(
                                ("ERROR: closing attribute statement mismatch; expected \"" +
                                toStringName(m_nodeStack.back()) +
                                "\"").c_str(), docPos);
                            retCode = false;
                        }
                        else
                        {
                            retCode = m_visitor.exit(match[1], false);
                            m_nodeStack.pop_back();
                        }
                    }
                }
                else if(((tmpPos = strchr(docPos, '<')) > docPos) && tmpPos)
                {
                    retCode = m_visitor.text(String(docPos, tmpPos));
                    docPos = tmpPos;
                }
                else if(std::regex_search(
                    docPos,
                    match,
                    regexXmlCDATA,
                    std::regex_constants::match_continuous))
                {
                    assert(!match.empty());
                    retCode = m_visitor.cdata(match[1]);
                }
                else if(std::regex_search(
                    docPos,
                    match,
                    regexXmlPI,
                    std::regex_constants::match_continuous))
                { // skip
                }
                else
                {
                    m_visitor.error(
                        "ERROR: invalid/unhandled statement or unexpected EOF",
                        docPos);
                    retCode = false;
                }

                if(retCode)
                {
                    assert(!match.empty() || (docPos && *docPos == '<'));
                    docPos = skipSpacesAndComments(match.empty() ?
                        docPos : match.suffix().first);
                    assert(docPos !=
                        (doc + strlen(doc)) || m_nodeStack.empty());
                }
            } while(retCode && !m_nodeStack.empty());
        }
#ifdef HO_SAX_CATCH_EXCEPTIONS
        catch(const std::exception& e)
        {
            m_visitor.error(
                (std::string("ERROR: std::exception ") + e.what()).c_str(),
                docPos);
            retCode = false;
        }
        catch(...)
        {
            m_visitor.error("ERROR: unknown exception", docPos);
            retCode = false;
        }
#endif // HO_SAX_CATCH_EXCEPTIONS

        return retCode;
    }

    // Convert to std string an element/attribute name
    static std::string toStringName(const String& it)
    {
        return toString(it, false, true);
    }
    // Convert to std string text
    static std::string toStringText(const String& it)
    {
        return fixEscapes(toString(it, true, true));
    }
    // Convert to std string CDATA
    static std::string toStringCdata(const String& it)
    {
        return toString(it, true, true);
    }
    // Convert to std string an attribute value
    static std::string toStringValue(const String& it)
    {
        return fixEscapes(toString(it, true, false));
    }
    // Obtain (row, col) pair of current position in C-string
    // Assume that '\r' not followed by '\n' means new line,
    // as in obsolete systems
    static std::pair<size_t, size_t> position(
        const char* const doc,
        const char* const docPos)
    {
        auto pos = std::make_pair<size_t, size_t>(1, 0);
        const char* last = doc;
        std::regex newline("\\r\\n|\\r|\\n");
        std::for_each(
            std::cregex_iterator(doc, docPos, newline),
            std::cregex_iterator(),
            [&](const std::cmatch& match)
            {
                ++pos.first;
                last = match.suffix().first;
            });
        assert(docPos >= last);
        pos.second = 1 + static_cast<size_t>(docPos - last);
        return pos;
    }

private: // functions
    static const std::string& getReName()
    {
        static const std::string name = "[a-zA-Z_][\\w\\.\\-]*";
        return name;
    }

    static const std::string& getReComment()
    {
        static const std::string comment = "(?:<!--(?:(?:[^-]|-(?!->))*)-->)";
        return comment;
    }

    const char* skipSpacesAndComments(const char* docPos)
    {
        static const std::regex spacesAndComments(
            "^(?:\\s+|\\s*" + getReComment() + "\\s*)+");
        std::cmatch match;
        if (std::regex_search(docPos, match, spacesAndComments,
                std::regex_constants::match_continuous))
        {
            docPos = match.suffix().first;
        }
        return docPos;
    }

    const char* skipDoctype(const char* docPos)
    {
        static const std::string id = "(?:\\w|#|-|,|\\(|\\)|\\*|\\?|\\+|\\|)+";
        static const std::string element =
            "(?:<!(?:ELEMENT|ATTLIST|NOTATION|ENTITY)\\s+"
            "(?:(?:" + id + "\\s*)|(?:\\\".*\\\"\\s*))+>)";
        static const std::regex regexDoctype(std::string("^<!DOCTYPE\\s+") +
            getReName() + "\\s*\\[" +
            "(?:" + getReComment() + "|" + element + "|\\s*)+" +
            "\\s*\\]>"
            );
        
        std::cmatch match;
        if (std::regex_search(docPos, match, regexDoctype,
                std::regex_constants::match_continuous))
        {
            docPos = skipSpacesAndComments(match.suffix().first);
        }

        return docPos;
    }

    static bool equalStrings(
        const String& s1,
        const String& s2)
    {
        const auto length1 = s1.second - s1.first;
        const auto length2 = s2.second - s2.first;
        assert(length1 >= 0 && length2 >= 0);

        return length1 == length2 &&
            !std::strncmp(s1.first, s2. first, static_cast<size_t>(length1));
    }

    static std::string fixEscapes(std::string&& s)
    {
        struct Esc {
            std::regex r;
            std::string str; };
        static const Esc esc[] = {
            { std::regex("&lt;"), "<" },
            { std::regex("&gt;"), ">" },
            { std::regex("&amp;"), "&", },
            { std::regex("&apos;"), "'" },
            { std::regex("&quot;"), "\"" } };

        std::for_each(esc, esc + sizeof(esc)/sizeof(*esc),
            [&s](const Esc& esc){
                s = std::regex_replace(s, esc.r, esc.str);});

        return s;
    }

    // Convert [begin, end) character sequence to std string
    // Normalization rules:
    // * value: space sequence -> one space
    // * text, CDATA: space sequence -> one space;
    //   remove surrounding spaces;
    static std::string toString(
        const String& it,
        bool normalize,
        bool removeSurrSpaces)
    {
        assert(it.first && it.second && it.first <= it.second);

        if (!normalize)
            return std::string(it.first, it.second);

        static const std::regex regexTrimmed(
            "(\\s+)?(\\S+(?:\\s+\\S+)*)?(\\s+)?");
        std::cmatch match;
        if (std::regex_match(it.first, it.second, match, regexTrimmed,
                std::regex_constants::match_continuous))
        {
            assert(!(!match[1].matched && !match[2].matched && match[3].matched) &&
                "Unexpected match, seen on VS2010");

            const String trimmed = std::make_pair(
                !removeSurrSpaces && match[1].matched ?
                    match[1].second - 1 :
                    (match[2].matched ? match[2].first : it.second),
                !removeSurrSpaces && match[3].matched ?
                    match[3].first + 1 :
                    (match[2].matched ? match[2].second : it.second));

            std::string s;
            std::regex_replace(
                std::back_inserter(s),
                trimmed.first,
                trimmed.second,
                std::regex("\\s+"),
                std::string(" "));

            return s;
        }
        
        return std::string(it.first, it.second);

/*        if (std::strspn(it.first, " \t\n\r") ==
            static_cast<size_t>(it.second - it.first))
        {
            return std::string(
                (!removeSurrSpaces && (it.second - it.first)) ? " " : "");
        }

        const char* pos = it.first;

        bool next = false;
        size_t nwBegin = std::strspn(pos, " \t\n\r");
        std::string s((!removeSurrSpaces && nwBegin > 0) ? " " : "");
        while (pos + nwBegin < it.second)
        {
            size_t nwEnd = std::strcspn(pos + nwBegin, " \t\n\r");

            if(!next)
                next = true;
            else
                s += ' ';
            
            s.append(pos + nwBegin, (pos + nwBegin + nwEnd < it.second) ?
                pos + nwBegin + nwEnd : it.second);

            nwBegin += nwEnd + std::strspn(pos + nwBegin + nwEnd, " \t\n\r");
        }

        if (!removeSurrSpaces && nwBegin > 0 && isspace(*(it.second-1)))
            s += ' ';

        return s;*/
    }

private: // data
    Visitor& m_visitor;

    std::vector<String> m_nodeStack;
};
} // headeronly

#ifdef HO_SAX_CATCH_EXCEPTIONS
#undef HO_SAX_CATCH_EXCEPTIONS
#endif // HO_SAX_CATCH_EXCEPTIONS

#endif // HO_SAX_HPP_
