//===--- OverloadedUnaryAndCheck.cpp - clang-tidy ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "OverloadedUnaryAndCheck.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/ASTContext.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace runtime {

void
OverloadedUnaryAndCheck::registerMatchers(ast_matchers::MatchFinder *Finder) {
  // Match unary methods that overload operator&.
  Finder->addMatcher(methodDecl(parameterCountIs(0), hasOverloadedOperatorName(
                                                         "&")).bind("overload"),
                     this);
  // Also match freestanding unary operator& overloads. Be careful not to match
  // binary methods.
  Finder->addMatcher(
      functionDecl(
          allOf(unless(methodDecl()),
                functionDecl(parameterCountIs(1),
                             hasOverloadedOperatorName("&")).bind("overload"))),
      this);
}

void OverloadedUnaryAndCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Decl = Result.Nodes.getNodeAs<FunctionDecl>("overload");
  diag(Decl->getLocStart(),
       "do not overload unary operator&, it is dangerous.");
}

} // namespace runtime
} // namespace tidy
} // namespace clang
