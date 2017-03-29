#include "FormulaExpression.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
FormulaExpression::FormulaExpression()
{
}

FormulaExpression::~FormulaExpression()
{
}

bool FormulaExpression::IsValue() const
{
    return false;
}

FormulaValueExpression::FormulaValueExpression(const Any& value_)
    : value(value_)
{
}

void FormulaValueExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

bool FormulaValueExpression::IsValue() const
{
    return true;
}

const Any& FormulaValueExpression::GetValue() const
{
    return value;
}

FormulaNegExpression::FormulaNegExpression(const std::shared_ptr<FormulaExpression>& exp_)
    : exp(exp_)
{
}

void FormulaNegExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

FormulaExpression* FormulaNegExpression::GetExp() const
{
    return exp.get();
}

FormulaNotExpression::FormulaNotExpression(const std::shared_ptr<FormulaExpression>& exp_)
    : exp(exp_)
{
}

void FormulaNotExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

FormulaExpression* FormulaNotExpression::GetExp() const
{
    return exp.get();
}

FormulaBinaryOperatorExpression::FormulaBinaryOperatorExpression(Operator op_, const std::shared_ptr<FormulaExpression>& lhs_, const std::shared_ptr<FormulaExpression>& rhs_)
    : op(op_)
    , lhs(lhs_)
    , rhs(rhs_)
{
}

FormulaBinaryOperatorExpression::Operator FormulaBinaryOperatorExpression::GetOperator() const
{
    return op;
}

FormulaExpression* FormulaBinaryOperatorExpression::GetLhs() const
{
    return lhs.get();
}

FormulaExpression* FormulaBinaryOperatorExpression::GetRhs() const
{
    return rhs.get();
}

void FormulaBinaryOperatorExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

int32 FormulaBinaryOperatorExpression::GetOperatorPriority() const
{
    switch (op)
    {
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
        return 1;

    case OP_PLUS:
    case OP_MINUS:
        return 2;

    case OP_LE:
    case OP_LT:
    case OP_GE:
    case OP_GT:
        return 3;

    case OP_EQ:
    case OP_NOT_EQ:
        return 4;

    case OP_AND:
        return 5;

    case OP_OR:
        return 6;

    default:
        DVASSERT("Invalid operator.");
        return 0;
    }
}

FormulaFunctionExpression::FormulaFunctionExpression(const String& name_, const Vector<std::shared_ptr<FormulaExpression>>& params_)
    : name(name_)
{
    params.reserve(params_.size());

    for (const std::shared_ptr<FormulaExpression>& exp : params_)
    {
        params.push_back(exp);
    }
}

const String& FormulaFunctionExpression::GetName() const
{
    return name;
}

const Vector<std::shared_ptr<FormulaExpression>>& FormulaFunctionExpression::GetParms() const
{
    return params;
}

void FormulaFunctionExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

FormulaFieldAccessExpression::FormulaFieldAccessExpression(const std::shared_ptr<FormulaExpression>& exp_, const String& fieldName_)
    : exp(exp_)
    , fieldName(fieldName_)
{
}

void FormulaFieldAccessExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

FormulaExpression* FormulaFieldAccessExpression::GetExp() const
{
    return exp.get();
}

const String& FormulaFieldAccessExpression::GetFieldName() const
{
    return fieldName;
}

FormulaIndexExpression::FormulaIndexExpression(const std::shared_ptr<FormulaExpression>& exp_, const std::shared_ptr<FormulaExpression>& indexExp_)
    : exp(exp_)
    , indexExp(indexExp_)
{
}

void FormulaIndexExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

FormulaExpression* FormulaIndexExpression::GetExp() const
{
    return exp.get();
}

FormulaExpression* FormulaIndexExpression::GetIndexExp() const
{
    return indexExp.get();
}
}
