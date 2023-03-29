#include "constraints.h" // EqualsRef
#include "parser/facade.h"
#include "synthesis/compiler.h"
#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

go_bandit([]() {
  describe("full", []() {
    it("program 1", [&]() {
      std::cout << "program 1\n";
      std::string input = "int y; int x = (1 + 2) * (y += 9);";
      parser p(input);
      auto presult = p.parse();
      std::cout << *presult.ast << '\n';

      compiler c;
      auto cresult = c.compile(presult.ast);

      std::cout << '\n' << input << '\n';
      std::cout << "data: " << cresult.data.size() << " bytes\n";

      std::cout << "code: \n";
      for (auto instruction : cresult.code) {
        std::cout << "> " << instruction << '\n';
      }
      std::cout << "end of program 1\n";
    });

    it("program 2", [&]() {
      std::cout << "program 2\n";
      std::string input = "\
        int x = 20; \
        int y = 10; \
        \
        if (x != y) { \
          y = 0; \
        } else { \
          y = 2; \
        } \
        \
        x = 100; \
      ";

      parser p(input);
      auto presult = p.parse();
      std::cout << *presult.ast << '\n';

      compiler c;
      auto cresult = c.compile(presult.ast);

      std::cout << '\n' << input << '\n';
      std::cout << "data: " << cresult.data.size() << " bytes\n";

      std::cout << "code: \n";
      for (size_t i = 0; i < cresult.code.size(); ++i) {
        auto instruction = cresult.code[i];
        std::cout << i << "> " << instruction << '\n';
      }
      std::cout << "end of program 2\n";
    });

    it("program 3", [&]() {
      std::cout << "program 3\n";
      std::string input = "\
        int x = 0; \
        int y = 1; \
        \
        while (x < 10) { \
          y *= 2; \
        } \
        \
        x = 100; \
      ";

      parser p(input);
      auto presult = p.parse();
      std::cout << *presult.ast << '\n';

      compiler c;
      auto cresult = c.compile(presult.ast);

      std::cout << '\n' << input << '\n';
      std::cout << "data: " << cresult.data.size() << " bytes\n";

      std::cout << "code: \n";
      for (size_t i = 0; i < cresult.code.size(); ++i) {
        auto instruction = cresult.code[i];
        std::cout << i << "> " << instruction << '\n';
      }

      std::cout << "statement boundaries\n";
      for (auto &boundary : cresult.metadata.statement_boundaries) {
        std::cout << "@" << boundary << '\n';
      }

      std::cout << "variables\n";
      for (auto &[addr, data] : cresult.metadata.variables) {
        // std::cout << entry->name << " @ " << addr << '\n';
        std::cout << "var " << data.name << " @ " << addr << '\n';
      }

      std::cout << "end of program 3\n";
    });

    it("program 4", [&]() {
      std::cout << "program 4\n";
      std::string input = "\
        int x = 0; \
        int vy = 1; \
        \
        loop:\
        if (x < 10) { \
          int x = 10; \
          goto calc; \
        } else { \
          goto end;\
        } \
        \
        calc:\
        vy *= 2;\
        goto loop;\
        \
        end:\
        x = 100; \
      ";

      parser p(input);
      auto presult = p.parse();
      std::cout << *presult.ast << '\n';

      compiler c;
      auto cresult = c.compile(presult.ast);

      std::cout << '\n' << input << '\n';
      std::cout << "data: " << cresult.data.size() << " bytes\n";

      std::cout << "code: \n";
      for (size_t i = 0; i < cresult.code.size(); ++i) {
        auto instruction = cresult.code[i];
        std::cout << i << "> " << instruction << '\n';
      }

      std::cout << "statement boundaries\n";
      for (auto &boundary : cresult.metadata.statement_boundaries) {
        std::cout << "@" << boundary << '\n';
      }

      std::cout << "variables\n";
      for (auto &[addr, data] : cresult.metadata.variables) {
        // std::cout << entry->name << " @ " << addr << '\n';
        std::cout << "var " << data.name << " declared at " << data.declared_at
                  << " @ " << addr << '\n';
      }

      std::cout << "end of program 4\n";
    });

    it("program 5", [&]() {
      std::cout << "program 5\n";
      std::string input = "\
        int x;\n \
        read x;\n \
        x += 1;\n \
        write x * 2;\n \
      ";

      parser p(input);
      auto presult = p.parse();
      std::cout << "success " << presult.success << '\n';
      std::cout << *presult.ast << '\n';

      std::cout << "message" << presult.message << '\n';

      compiler c;
      auto cresult = c.compile(presult.ast);

      std::cout << '\n' << input << '\n';
      std::cout << "data: " << cresult.data.size() << " bytes\n";

      std::cout << "code: \n";
      for (size_t i = 0; i < cresult.code.size(); ++i) {
        auto instruction = cresult.code[i];
        auto line = cresult.metadata.source_line_map[i];
        std::cout << i << "> " << instruction << " <- " << line << '\n';
      }

      std::cout << "statement boundaries\n";
      for (auto &boundary : cresult.metadata.statement_boundaries) {
        std::cout << "@" << boundary << '\n';
      }

      std::cout << "variables\n";
      for (auto &[addr, data] : cresult.metadata.variables) {
        // std::cout << entry->name << " @ " << addr << '\n';
        std::cout << "var " << data.name << " declared at " << data.declared_at
                  << " @ " << addr << '\n';
      }

      std::cout << "end of program 5\n";
    });

    it("program 6", [&]() {
      std::cout << "program 6\n";
      std::string input = "abobora";

      parser p(input);
      auto presult = p.parse();
      std::cout << "success " << presult.success << '\n';
      std::cout << "message" << presult.message << '\n';
      std::cout << "end of program 6\n";
    });
  });
});
