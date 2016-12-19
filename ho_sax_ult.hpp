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
    This is a part of header-only (see: wiki Header-only) tiny utils library.

    ULT for the SAX parser.
*/

#ifndef HO_SAX_ULT_HPP_
#define HO_SAX_ULT_HPP_

#include <iostream>

namespace headeronly
{
namespace xmlsaxult
{
/// Auxiliary input xml strings,for negative testing
static const char* const InvalidStringNoValue =
"<Elem1 par=\" val \" par6="   ">\n\
Preamble\n\
<FirstElement></FirstElement>   \n\
</Elem1> \n\
";

static const char* const InvalidStringDupAttr =
"<Elem1 par=\"val1\" par3=\"val2\" par=\"val3\">\n\
Preamble\n\
<FirstElement></FirstElement>   \n\
</Elem1> \n\
";

static const char* const InvalidStringClosingElemNoMatch =
"<Elem1>Preamble<FirstElement></FirstElement></BadElem>";


// ULT class
struct Data
{
    bool positive;
    const char* input;
    // positive: expected output string, negative: error position in input string
    const char* outputOrPos;
};

// ULT input
static const Data data[]=
{
{
true, // empty document
"",
""
},
{
true, // empty document
"  \r  \t  \n  \r\n  \t",
""
},
{
true, // empty document
"  <!-- oneline comment  --> <!-- oneline comment  --> ",
""
},
{
true, // empty document
" <!-- oneline comment  --> \n\
<?xml version=\"1.0\" encoding=\"utf-8\"?> \n\
<!-- oneline comment  --> \n\
",
""
},
{
true, // generic - trivial, no indentations, no extra spaces
"<RootElement>"
"<ManyAttributes param=\"value\" par2=\"abcd\" par3=\"\" par4=\" a b c \">"
"Preamble"
"<FirstElement>\n\
Some Text\n\
</FirstElement>\n\
</ManyAttributes>\n\
</RootElement>",
"<RootElement>"
"<ManyAttributes param=\"value\" par2=\"abcd\" par3=\"\" par4=\" a b c \">"
"Preamble"
"<FirstElement>"
"Some Text"
"</FirstElement>"
"</ManyAttributes>"
"</RootElement>"
},
{
true, // generic simple
"  \n\
 \t\r\n \n\
<RootElement>\n\
 \t\r\n <!-- --> \n\
 \t\r\n <!-- \n\
 \t\r\n --> \n\
    <Attributes1 param1=\"value\">\n\
    \t\r\n <!-- oneline comment  --> \n\
    </Attributes1>\n\
    \t\r\n <!--  < > & ; <aa bb=\"2\"/> -->  \n\
       <!--escape <code/> & is allowed in comments--> \n\
    \t\r\n  <!-- \n\
        \t\r\n multiline \t\r\n \n\
        \t\r\n comment \t\r\n \n\
        --> \n\
    <Attributes2   param2  =  \" value2 \"  >\n\
    </Attributes2>   \n\
    <Attributes3 \t \r \n \t param3=\"   value  3    \"/>   \n\
    <Attributes4    param1  =  \"  value   \"  />   \n\
    <Attributes5 pArAm123.-_=\"value\"/>   \n\
    <Attributes6 param1=\"value1\" \n  \r \t param2=\"value2\" param3=\"value3\">   \n\
    </Attributes6>   \n\
    <Attributes7  param1 = \" value1 \" param2 = \"  value2  \" >\n\
    </Attributes7>\n\
    <Attributes8 param1=\"value1\" param2=\"value2\"/>\n\
    <Attributes9    param1 = \" value1 \"   param2  \t \n\r =  \t \n\r \"value2\"   />\n\
    <Escapes v-_1=\"&lt;\" v_-2=\"&gt;\" v3=\"&amp;\" v4=\"&apos;\" v5=\"&quot;\"> \n\
    </Escapes> \n\
    <Import Project=\"$(VCTargets)\\Platforms\\*.targets\" Condition=\"Exists('$(VCPath)//Platforms/Win32/ImportBefore')\" /> \n\
    <Access Condition=\"'$(Track)' == ''\">true</Access> \n\
</RootElement>  \n\
",
"<RootElement>"
"<Attributes1 param1=\"value\">"
"</Attributes1>"
"<Attributes2 param2=\" value2 \">"
"</Attributes2>"
"<Attributes3 param3=\" value 3 \"/>"
"<Attributes4 param1=\" value \"/>"
"<Attributes5 pArAm123.-_=\"value\"/>"
"<Attributes6 param1=\"value1\" param2=\"value2\" param3=\"value3\">"
"</Attributes6>"
"<Attributes7 param1=\" value1 \" param2=\" value2 \">"
"</Attributes7>"
"<Attributes8 param1=\"value1\" param2=\"value2\"/>"
"<Attributes9 param1=\" value1 \" param2=\"value2\"/>"
"<Escapes v-_1=\"<\" v_-2=\">\" v3=\"&\" v4=\"'\" v5=\"\"\">"
"</Escapes>"
"<Import Project=\"$(VCTargets)\\Platforms\\*.targets\" Condition=\"Exists('$(VCPath)//Platforms/Win32/ImportBefore')\"/>"
"<Access Condition=\"'$(Track)' == ''\">true</Access>"
"</RootElement>"
},
{ // generic
true,
"<?xml version=\"1.0\" encoding=\"utf-8\"?>  \n\
 <RootElement>\n\
    <ManyAttributes param=\"value\" par2 = \"abcd\" par3=\"\"  par4=\"  a  b  c  \"  \n\
        par5=\" \" par6=\"  \" par7=\"   \"  par8=\"    \"  par9=\"  \t\t  \">\n\
      Preamble\n\
      <FirstElement>            \n\
        Some Text               \n\
      </FirstElement  \t  >   \n\
      <SecondElement param2  =   \"something![CDATA[]]>\"  param3=\">'some>'thing>'\" \n\
        param4=\"\\>'some\\>'thing\\>'\">  \n\
        Pre-Text <Inline>Inlined text</Inline> Post-text.  \n\
      </SecondElement   >     \n\
      Inamble  \t \r \n \t \n\
      <!-- asd \n \t  \r sdf --> \n\
      Lorem ipsum dolor sit amet, \t \r \n \t \n\
      <!---->  \n\
      <!-- -->  \n\
      <!-- < <! <!- <!-- -- -->  \n\
      <!-- -- > - -> --> \n\
      \t \r \n \t consectetur adipiscing elit.    \n\
      <!-- <>&; \t \r \n \n\
        dolor \n\
        amen --> \n\
       \t \r \n \t \n\
          Multiline and \t \t \r\r \n\
          very fine \t \t \r\r \n \r \n\n \n\
          wow text \n\
      <ThirdElement par3   =    \" bcd\"  \n\
    par4 = \"xyz \"        \t \r \n \t          \n\
          par5 = \" pqrst \" \t \r \n \t \n\
            \n\
              \n\
      />  \n\
        \n\
      <FourthElement/>  \n\
        \n\
      <MULTILINE_EMPTY_ATTRIBUTE attr=\"  \t \n\r \r\n \n\
      \t \r \n \t \n \n\
       \t \r \n \n\
       \n\
      \"/>  \n\
      <MULTILINE_ATTRIBUTE attr=\"  \t \n\r \r\n Lorem ipsum dolor sit amet, \n\
      \t \r \n \t \n consectetur \n\
       adipiscing \t \r \n \n\
        elit.   \n\
      \"/>  \n\
        \n\
      <FifthElement    />  \n\
        \n\
        00 ff 1a bf  00 ff 1a bf  00 ff 1a bf  00 ff 1a bf \n\
        11 bb 7f a1  11 bb 7f a1  11 bb 7f a1  11 bb 7f a1 \n\
      <SixthElement><SeventhElement><EigthElement></EigthElement></SeventhElement></SixthElement>  \n\
        \n\
        <NinenthElement v=\"   a \n\
     \t \r \n \t             b c  \n\
     d   \n\
     ef  \t \r \n \t \"/>   \n\
     <TEXT_AND_CDATA><![CDATA[]]></TEXT_AND_CDATA> \n\
     <TEXT_AND_CDATA><![CDATA[a]]></TEXT_AND_CDATA> \n\
     <TEXT_AND_CDATA> \n\
         <![CDATA[]]>  \n\
         <![CDATA[   ]]>  \n\
         <![CDATA[   \t \n \r\n \n\r \n\
           ]]> \n\
         <![CDATA[  ] ]] ]> ]]>  \n\
         <![CDATA[<div><p><greeting>Hello&;world!</greeting></p></div>]]> \n\
         <![CDATA[  <div>  <p>\t<greeting> Hello&;world! \t</greeting> </p>\t</div>\t    ]]> \n\
     </TEXT_AND_CDATA> \n\
      Postamble   \n\
        \n\
    </ManyAttributes>  \n\
    <Nested val=\"1\"><Nested val=\"2\">Text1<Nested val=\"3\">\n\
    <Nested val=\"4\"> <Nested val=\"5\"> <Nested val=\"6\"> Text 2 \n\
    </Nested></Nested></Nested>Text3</Nested>\n\
    </Nested>Text4</Nested>\n\
    Last Text > &gt; - -> --> &lt;!-- \n\
</RootElement>  \n\
",
"<RootElement>"
"<ManyAttributes param=\"value\" par2=\"abcd\" par3=\"\" par4=\" a b c \" par5=\" \" par6=\" \" par7=\" \" par8=\" \" par9=\" \">"
"Preamble"
"<FirstElement>"
"Some Text"
"</FirstElement>"
"<SecondElement param2=\"something![CDATA[]]>\" param3=\">'some>'thing>'\" param4=\"\\>'some\\>'thing\\>'\">"
"Pre-Text"
"<Inline>"
"Inlined text"
"</Inline>"
"Post-text."
"</SecondElement>"
"Inamble"
"Lorem ipsum dolor sit amet,"
"consectetur adipiscing elit."
"Multiline and very fine wow text"
"<ThirdElement par3=\" bcd\" par4=\"xyz \" par5=\" pqrst \"/>"
"<FourthElement/>"
"<MULTILINE_EMPTY_ATTRIBUTE attr=\" \"/>"
"<MULTILINE_ATTRIBUTE attr=\" Lorem ipsum dolor sit amet, consectetur adipiscing elit. \"/>"
"<FifthElement/>"
"00 ff 1a bf 00 ff 1a bf 00 ff 1a bf 00 ff 1a bf"
" 11 bb 7f a1 11 bb 7f a1 11 bb 7f a1 11 bb 7f a1"
"<SixthElement><SeventhElement><EigthElement></EigthElement></SeventhElement></SixthElement>"
"<NinenthElement v=\" a b c d ef \"/>"
"<TEXT_AND_CDATA><![CDATA[]]></TEXT_AND_CDATA>"
"<TEXT_AND_CDATA><![CDATA[a]]></TEXT_AND_CDATA>"
"<TEXT_AND_CDATA>"
"<![CDATA[]]>"
"<![CDATA[]]>"
"<![CDATA[]]>"
"<![CDATA[] ]] ]>]]>"
"<![CDATA[<div><p><greeting>Hello&;world!</greeting></p></div>]]>"
"<![CDATA[<div> <p> <greeting> Hello&;world! </greeting> </p> </div>]]>"
"</TEXT_AND_CDATA>"
"Postamble"
"</ManyAttributes>"
"<Nested val=\"1\"><Nested val=\"2\">Text1<Nested val=\"3\">"
"<Nested val=\"4\"><Nested val=\"5\"><Nested val=\"6\">Text 2"
"</Nested></Nested></Nested>Text3</Nested>"
"</Nested>Text4</Nested>"
"Last Text > > - -> --> <!--"
"</RootElement>"
},
{ // DTD - makes no sense, but should be parsed and skipped
true,
"<?xml version=\"1.0\" encoding=\"utf-8\" ?> \n\
<!-- abd <aa bb/> \" \"& --> \n\
 <!DOCTYPE doc [ \n\
  <!ELEMENT doc (el)*> \n\
  <!-- \n\
     the optional \"type\" attribute \n\
   --> \n\
  <!ATTLIST el \n\
    type  NOTATION ( \n\
      type-something ) #IMPLIED> \n\
      \n\
  <!ELEMENT img ANY> \n\
  <!-- comment --> \n\
  <!ELEMENT (#PCDATA | a | b |c|d |e, x, y)*+?> \n\
  <!-- \n\
     another comment \n\
   --> \n\
  <!NOTATION type-images       PUBLIC \"image/png\" \n\
      \"image/bmp\"> \n\
      \n\
      <!ENTITY example1 \"example 1 <!-- --> \" \"]> example 2 ]>\"> \n\
  <!-- comment --> \n\
  <!ENTITY example2 SYSTEM \"example/2\" NDATA type-images> \n\
  <!-- comment --> \n\
 ]> \n\
\n\
<Root-Element  type = \"int\"> \n\
</Root-Element > \n\
",
"<Root-Element type=\"int\">"
"</Root-Element>"
},
{ // XML namespaces and special attributes
true,
"<Root> \n\
    <title xml:lang=\"en-US\">Special attribute</title> \n\
    <title  available = \"true\"  xml:lang = \"de-DE\" > Special attribute 2 </title> \n\
    <title  xml:lang = \"de-DE\"  available = \"true\" > Special attribute 2 </title> \n\
    <assembly xmlns=\"urn:schemas-com:asm.1\" manifestVersion=\"2.0\"> \n\
      <diff xmlns=\"urn:schemas-com:asm.2\">  \n\
      </diff>  \n\
      <book xmlns:hr=\"http://rumcays.org/id?=123\" xml:available=\"true\"> \n\
       <hr:author key=\"BS\"> \n\
        <hr:name>Billy Silly</hr:name> \n\
        <hr:born xmlns:place=\"http://hellsburg.org/main.html\">1666-02-31</hr:born> \n\
       </hr:author> \n\
      </book> \n\
    </assembly>  \n\
</Root> \n\
",
"<Root>"
"<title xml:lang=\"en-US\">Special attribute</title>"
"<title available=\"true\" xml:lang=\"de-DE\">Special attribute 2</title>"
"<title xml:lang=\"de-DE\" available=\"true\">Special attribute 2</title>"
"<assembly xmlns=\"urn:schemas-com:asm.1\" manifestVersion=\"2.0\">"
"<diff xmlns=\"urn:schemas-com:asm.2\">"
"</diff>"
"<book xmlns:hr=\"http://rumcays.org/id?=123\" xml:available=\"true\">"
"<hr:author key=\"BS\">"
"<hr:name>Billy Silly</hr:name>"
"<hr:born xmlns:place=\"http://hellsburg.org/main.html\">1666-02-31</hr:born>"
"</hr:author>"
"</book>"
"</assembly>"
"</Root>"
},
{ false, // negative test - no value assigned to attribute
InvalidStringNoValue,
InvalidStringNoValue + 0 // whole element
},
{ false, // negative test - duplicate attributes
InvalidStringDupAttr,
InvalidStringDupAttr + 0 // whole element containing particular attribute
},
{ false, // negative test - invalid closing element name
InvalidStringClosingElemNoMatch,
InvalidStringClosingElemNoMatch + 44 // closing element
}
}
;

/// SAX ULT
struct XmlSaxULT : XmlSax::Visitor
{
    virtual bool enter(const XmlSax::String& element, bool isEmptyElementTag)
    {
        closePreviousElement();

        m_parsed += tee("<" + XmlSax::toStringName(element));

        m_ClosePreviousElement = !isEmptyElementTag;

        return true;
    }
    virtual bool exit(const XmlSax::String& node, bool isEmptyElementTag)
    {
        closePreviousElement();

        m_parsed += tee(isEmptyElementTag ?
            "/>" : "</" + XmlSax::toStringName(node) + ">" );

        return true;
    }
    virtual bool attribute(
        const XmlSax::String& name,
        const XmlSax::String& value)
    {
        m_parsed += tee(" " + XmlSax::toStringName(name) + "=\"" +
            XmlSax::toStringValue(value) + "\"");

        return true;
    }
    virtual bool text(const XmlSax::String& text)
    {
        closePreviousElement();

        m_parsed += tee(XmlSax::toStringText(text));

        return true;
    }
    virtual bool cdata(const XmlSax::String& text)
    {
        closePreviousElement();

        m_parsed += "<![CDATA[" + tee(XmlSax::toStringCdata(text)) + "]]>";

        return true;
    }
    virtual void error(const char* info, const char* docPos)
    {
        assert(docPos && !m_ErrorPosInString);
        m_ErrorPosInString = docPos;

        if (m_PositiveTest)
        {
            // Show error message for positive tests only
            const auto position = XmlSax::position(m_DocStart, docPos);
            std::cout << std::endl << "SAX PARSE " << std::string(info) <<
                " at (" << position.first << ", " << position.second << ")" <<
                ":\n" << docPos << std::endl;
        }
    }
    virtual bool validate()
    {
        return true;
    }

    void closePreviousElement()
    {
        if (!m_ClosePreviousElement)
            return;
        m_parsed += tee(">");
        m_ClosePreviousElement = false;
    }
    // Override it if you want to split the output
    virtual std::string tee(const std::string& s)
    {
        return s;
    }

    bool diff(const std::string& reference, const std::string& input)
    {
        for(size_t n=0; n < std::min(reference.size(), input.size()); ++n)
        {
            if (reference[n] != input[n])
            {
                std::cout
                    << "\n================================================\n"
                    << "Diff at position " << n << std::endl
                    << "================================================\n"
                    << "there is:\n " << input.c_str() + n << std::endl
                    << "================================================\n"
                    << "there should be:\n" << reference.c_str() + n << std::endl
                    << "================================================\n";
                return true;
            }
        }

        if (reference.size() != input.size())
        {
            std::cout << "Different size of string" << std::endl;
            return true;
        }

        return false;
    }

    XmlSaxULT(bool enableAssertions):
        m_ClosePreviousElement(false),
        m_ErrorPosInString(nullptr),
        m_EnableAssertions(enableAssertions),
        m_PositiveTest(true),
        m_DocStart(nullptr)
    {}

    bool run()
    {
        bool allPassed = true;
        for(size_t n = 0; n < sizeof(data)/sizeof(*data); ++n)
        {
            m_parsed.clear();
            m_ClosePreviousElement = false;
            m_ErrorPosInString = nullptr;
            m_PositiveTest = data[n].positive;
            m_DocStart = data[n].input;

            tee("\n====== Expected ======\n");
            tee(data[n].outputOrPos);
            tee("\n====== Actual ======\n");

            XmlSax sax(*this);

            bool passed = false;
            const char* failureDesc = nullptr;

            if(sax.parse(m_DocStart))
            {
                if (m_PositiveTest)
                {
                    passed = !diff(data[n].outputOrPos, m_parsed);
                    failureDesc = "positive test: output != expected pattern.";
                }
                else
                {
                    passed = false;
                    failureDesc = "negative test: expected was parsing error.";
                }
            }
            else
            {
                if (m_PositiveTest)
                {
                    passed = false;
                    failureDesc = "positive test: parsing error.";
                }
                else
                {
                    passed = data[n].outputOrPos == m_ErrorPosInString;
                    failureDesc = "negative test: parsing error - wrong position.";
                }
            }

            allPassed = allPassed && passed;

            tee("\n====================\n");
            std::cout << "XmlSaxULT #" << n << "  " <<
                (passed ? "passed" : "failed ") <<
                (passed ? "" : failureDesc ) << std::endl;
            assert(!m_EnableAssertions || passed);
        }

        return allPassed;
    }

    std::string m_parsed;
    bool m_ClosePreviousElement;
    const char* m_ErrorPosInString;
    bool m_EnableAssertions;
    // for calbacks
    bool m_PositiveTest;
    const char* m_DocStart;
};

// ULT auto-run
#ifdef XmlSaxULT_Run
#undef XmlSaxULT_Run
struct XmlSaxULTAutorun : XmlSaxULT
{
    virtual std::string tee(const std::string& s)
    {
        if (m_teeEnabled) std::cout << s;
        return s;
    }

    XmlSaxULTAutorun(bool enableAssertions, bool teeEnabled):
        XmlSaxULT(enableAssertions),
        m_teeEnabled(teeEnabled)
    {
        const bool passed = run();
        m_teeEnabled = true; // for printing summary
        throw std::runtime_error(
            tee(std::string("\nXmlSaxULT ") + (passed ? "passed" : "failed") +
            ". Remember to disable this before providing the code to production.\n"));

    }

    bool m_teeEnabled; // set to true to write pattern/parsed data to std out
};
static const XmlSaxULTAutorun xmlSaxULTAutorun(false, false);
#endif // XmlSaxULT_Run

} // xmlsaxult
} // headeronly

#endif // HO_SAX_ULT_HPP_
