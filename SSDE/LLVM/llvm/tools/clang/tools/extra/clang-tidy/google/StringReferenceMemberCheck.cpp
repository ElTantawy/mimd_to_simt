//===--- StringReferenceMemberCheck.cpp - clang-tidy ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "StringReferenceMemberCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ASTContext.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace runtime {

void StringReferenceMemberCheck::registerMatchers(
    ast_matchers::MatchFinder *Finder) {
  // Look for const references to std::string or ::string.
  auto String = anyOf(recordDecl(hasName("::std::basic_string")),
                      recordDecl(hasName("::string")));
  auto ConstString = qualType(isConstQualified(), hasDeclaration(String));

  // Ignore members in template instantiations.
  Finder->addMatcher(fieldDecl(hasType(references(ConstString)),
                               unless(isInstantiated())).bind("member"),
                     this);
}

void
StringReferenceMemberCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Member = Result.Nodes.getNodeAs<FieldDecl>("member");
  diag(Member->getLocStart(), "const string& members are dangerous. It is much "
                              "better to use alternatives, such as pointers or "
                              "simple constants.");
}

} // namespace runtime
} // namespace tidy
} // namespace clang
