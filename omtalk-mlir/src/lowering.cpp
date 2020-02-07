#include <memory>

#include "llvm/ADT/Sequence.h"
#include "mlir/Conversion/AffineToStandard/AffineToStandard.h"
#include "mlir/Conversion/LoopToStandard/ConvertLoopToStandard.h"
#include "mlir/Conversion/StandardToLLVM/ConvertStandardToLLVM.h"
#include "mlir/Conversion/StandardToLLVM/ConvertStandardToLLVMPass.h"
#include "mlir/Dialect/AffineOps/AffineOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LoopOps/LoopOps.h"
#include "mlir/Dialect/StandardOps/Ops.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "omtalk/dialect.hpp"

namespace {

struct AddOpLowering : public mlir::OpRewritePattern<omtalk::IAddOp> {
  using OpRewritePattern<omtalk::IAddOp>::OpRewritePattern;

  mlir::PatternMatchResult matchAndRewrite(
      omtalk::IAddOp op, mlir::PatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<mlir::AddIOp>(op, op.recv(), op.rhs());
    return matchSuccess();
  }
};

struct ConstantOpLowering : public mlir::OpRewritePattern<omtalk::ConstantOp> {
  using OpRewritePattern<omtalk::ConstantOp>::OpRewritePattern;

  mlir::PatternMatchResult matchAndRewrite(
      omtalk::ConstantOp op, mlir::PatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<mlir::ConstantOp>(op, op.valueAttr());
    return matchSuccess();
  }
};

struct ReturnOpLowering : public mlir::OpRewritePattern<omtalk::ReturnOp> {
  using OpRewritePattern<omtalk::ReturnOp>::OpRewritePattern;

  mlir::PatternMatchResult matchAndRewrite(
      omtalk::ReturnOp op, mlir::PatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<mlir::ReturnOp>(op, op.getOperand());
    return matchSuccess();
  }
};

}  // namespace

namespace omtalk {

struct OmtalkLoweringPass : public mlir::FunctionPass<OmtalkLoweringPass> {
  void runOnFunction() final;
};

void OmtalkLoweringPass::runOnFunction() {
  auto function = getFunction();

  if (function.getName() != "run") {
    return;
  }

  mlir::ConversionTarget target(getContext());

  target.addLegalDialect<mlir::StandardOpsDialect>();
  target.addIllegalDialect<omtalk::Dialect>();

  mlir::OwningRewritePatternList patterns;
  patterns.insert<AddOpLowering, ConstantOpLowering, ReturnOpLowering>(
      &getContext());

  if (failed(applyPartialConversion(getFunction(), target, patterns))) {
    signalPassFailure();
  }
}

/// Create a pass for lowering operations in the `Affine` and `Std` dialects,///
/// for a subset of the Toy IR (e.g. matmul).
std::unique_ptr<mlir::Pass> createLowerPass() {
  return std::make_unique<OmtalkLoweringPass>();
}

}  // namespace omtalk

namespace omtalk {
struct ToLlvmLoweringPass : public mlir::ModulePass<ToLlvmLoweringPass> {
  void runOnModule() final;
};

void ToLlvmLoweringPass::runOnModule() {
  mlir::ConversionTarget target(getContext());
  target.addLegalDialect<mlir::LLVM::LLVMDialect>();
  target.addLegalOp<mlir::ModuleOp, mlir::ModuleTerminatorOp>();

  mlir::LLVMTypeConverter typeConverter(&getContext());

  mlir::OwningRewritePatternList patterns;
  populateAffineToStdConversionPatterns(patterns, &getContext());
  populateLoopToStdConversionPatterns(patterns, &getContext());
  populateStdToLLVMConversionPatterns(typeConverter, patterns);

  auto module = getModule();
  if (failed(applyFullConversion(module, target, patterns, &typeConverter)))
    signalPassFailure();
}

std::unique_ptr<mlir::Pass> createToLlvmLoweringPass() {
  return std::make_unique<ToLlvmLoweringPass>();
}

}  // namespace omtalk