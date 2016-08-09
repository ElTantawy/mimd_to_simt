//===--- ExplicitConstructorCheck.cpp - clang-tidy ------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ExplicitConstructorCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void ExplicitConstructorCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(constructorDecl(unless(isInstantiated())).bind("ctor"),
                     this);
}

// Looks for the token matching the predicate and returns the range of the found
// token including trailing whitespace.
SourceRange FindToken(const SourceManager &Sources, LangOptions LangOpts,
                      SourceLocation StartLoc, SourceLocation EndLoc,
                      bool (*Pred)(const Token &)) {
  if (StartLoc.isMacroID() || EndLoc.isMacroID())
    return SourceRange();
  FileID File = Sources.getFileID(Sources.getSpellingLoc(StartLoc));
  StringRef Buf = Sources.getBufferData(File);
  const char *StartChar = Sources.getCharacterData(StartLoc);
  Lexer Lex(StartLoc, LangOpts, StartChar, StartChar, Buf.end());
  Lex.SetCommentRetentionState(true);
  Token Tok;
  do {
    Lex.LexFromRawLexer(Tok);
    if (Pred(Tok)) {
      Token NextTok;
      Lex.LexFromRawLexer(NextTok);
      return SourceRange(Tok.getLocation(), NextTok.getLocation());
    }
  } while (Tok.isNot(tok::eof) && Tok.getLocation() < EndLoc);

  return SourceRange();
}

bool isStdInitializerList(QualType Type) {
  if (const RecordType *RT = Type.getCanonicalType()->getAs<RecordType>()) {
    if (ClassTemplateSpecializationDecl *Specialization =
            dyn_cast<ClassTemplateSpecializationDecl>(RT->getDecl())) {
      ClassTemplateDecl *Template = Specialization->getSpecializedTemplate();
      // First use the fast getName() method to avoid unnecessary calls to the
      // slow getQualifiedNameAsString().
      return Template->getName() == "initializer_list" &&
             Template->getQualifiedNameAsString() == "std::initializer_list";
    }
  }
  return false;
}

void ExplicitConstructorCheck::check(const MatchFinder::MatchResult &Result) {
  const CXXConstructorDecl *Ctor =
      Result.Nodes.getNodeAs<CXXConstructorDecl>("ctor");
  // Do not be confused: isExplicit means 'explicit' keyword is present,
  // isImplicit means that it's a compiler-generated constructor.
  if (Ctor->isOutOfLine() || Ctor->isImplicit() || Ctor->isDeleted() ||
      Ctor->getNumParams() == 0 || Ctor->getMinRequiredArguments() > 1)
    return;

  bool takesInitializerList = isStdInitializerList(
      Ctor->getParamDecl(0)->getType().getNonReferenceType());
  if (Ctor->isExplicit() &&
      (Ctor->isCopyOrMoveConstructor() || takesInitializerList)) {
    auto isKWExplicit = [](const Token &Tok) {
      return Tok.is(tok::raw_identifier) &&
             Tok.getRawIdentifier() == "explicit";
    };
    SourceRange ExplicitTokenRange =
        FindToken(*Result.SourceManager, Result.Context->getLangOpts(),
                  Ctor->getOuterLocStart(), Ctor->getLocEnd(), isKWExplicit);
    StringRef ConstructorDescription;
    if (Ctor->isMoveConstructor())
      ConstructorDescription = "move";
    else if (Ctor->isCopyConstructor())
      ConstructorDescription = "copy";
    else
      ConstructorDescription = "initializer-list";

    DiagnosticBuilder Diag =
        diag(Ctor->getLocation(),
             "%0 constructor should not be declared explicit")
        << ConstructorDescription;
    if (ExplicitTokenRange.isValid()) {
      Diag << FixItHint::CreateRemoval(
          CharSourceRange::getCharRange(ExplicitTokenRange));
    }
    return;
  }

  if (Ctor->isExplicit() || Ctor->isCopyOrMoveConstructor() ||
      takesInitializerList)
    return;

  SourceLocation Loc = Ctor->getLocation();
  diag(Loc, "single-argument constructors must be explicit")
      << FixItHint::CreateInsertion(Loc, "explicit ");
}

} // namespace tidy
} // namespace clang
