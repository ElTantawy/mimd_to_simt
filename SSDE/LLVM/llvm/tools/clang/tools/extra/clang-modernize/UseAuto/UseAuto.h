//===-- UseAuto/UseAuto.h - Use auto type specifier -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file provides the definition of the UseAutoTransform class
/// which is the main interface to the use-auto transform that replaces
/// type specifiers with the special C++11 'auto' type specifier in certain
/// situations.
///
//===----------------------------------------------------------------------===//

#ifndef CLANG_MODERNIZE_USE_AUTO_H
#define CLANG_MODERNIZE_USE_AUTO_H

#include "Core/Transform.h"
#include "llvm/Support/Compiler.h"

/// \brief Subclass of Transform that transforms type specifiers for variable
/// declarations into the special C++11 'auto' type specifier for certain cases:
/// * Iterators of std containers.
/// * More to come...
///
/// Other uses of the auto type specifier as outlined in C++11 [dcl.spec.auto]
/// p2 are not handled by this transform.
class UseAutoTransform : public Transform {
public:
  UseAutoTransform(const TransformOptions &Options)
      : Transform("UseAuto", Options) {}

  /// \see Transform::run().
  virtual int apply(const clang::tooling::CompilationDatabase &Database,
                    const std::vector<std::string> &SourcePaths) override;
};

#endif // CLANG_MODERNIZE_USE_AUTO_H
