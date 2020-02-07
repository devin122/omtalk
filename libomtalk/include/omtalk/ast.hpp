#ifndef OMRTALK_AST_HPP
#define OMRTALK_AST_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace omtalk {

struct Location {
  std::size_t line = 1;
  std::size_t column = 1;
  std::size_t offset = 0;
  std::string filename = "";
};

struct LocationRange {
  Location start;
  Location end;
};

namespace ast {

enum class Kind {
  UNKNOWN,
  SYMBOL,
  COMMENT,
  ASSIGNMENT,
  CLAZZ,
  METHOD,
  SIGNATURE,
  BODY,
  STRING,
  UNARY_METHOD,
  INTEGER,
  BINARY_METHOD,
  KEYWORD_METHOD,
  EXPRESSION
};

/// Base AST element type.
///
class Node {
 public:
  constexpr Node(Kind k) : _kind(k) {}

  virtual ~Node() = default;

  constexpr Kind kind() const noexcept { return _kind; }

 private:
  Kind _kind;
};

template <Kind K>
class NodeInstance : public Node {
 public:
  static const constexpr Kind KIND = K;

  constexpr NodeInstance() : Node(K) {}
};

template <typename T>
T* node_cast(Node* node) {
  static_assert(std::is_base_of_v<NodeInstance<T::KIND>, T>);
  assert(node->kind() == T::KIND);
  return static_cast<T*>(node);
}

using NodePtr = std::shared_ptr<Node>;
using NodePtrVec = std::vector<NodePtr>;

/// Root AST object.
///
class Root {
 public:
  auto begin() { return _exprs.begin(); }

  auto end() { return _exprs.end(); }

  bool empty() const { return _exprs.empty(); }

  NodePtrVec& exprs() { return _exprs; }

  const NodePtrVec& exprs() const { return _exprs; }

 private:
  NodePtrVec _exprs;
};

constexpr bool is_operator(char c) {
  switch (c) {
    case '&':  // and
    case '|':  // or
    case '~':  // not
    case '*':  // star / mult
    case '@':  // at
    case ',':
    case '+':
    case '-':
    case '%':  // per
    case '/':  // div
    case '<':
    case '>':
    case '=':
    case '\\':  // mod
      return true;
    default:
      return false;
  }
}

class Symbol : public NodeInstance<Kind::SYMBOL> {
 public:
  Symbol() = default;

  Symbol(std::string value) : _location(), _value(value) {}

  Symbol(LocationRange location, std::string value)
      : _location(location), _value(value) {}

  Symbol(Location start, Location end, std::string value)
      : _location{start, end}, _value(value) {}

  Symbol(const Symbol& other) = default;

  Symbol(Symbol&& other) = default;

  Symbol& operator=(const Symbol&) = default;

  Symbol& operator=(Symbol&&) = default;

  const std::string& str() const { return _value; }

  const char* c_str() const { return str().c_str(); }

  const LocationRange& location() const { return _location; }

 private:
  LocationRange _location;
  std::string _value;
};

using SymbolPtr = std::shared_ptr<Symbol>;

using SymbolList = std::vector<SymbolPtr>;

enum class SymbolKind { IDENTIFIER, KEYWORD, OPERATOR };

inline SymbolKind symbol_kind(const std::string& str) {
  SymbolKind kind;
  if (str.length() == 1 && is_operator(str[0])) {
    kind = SymbolKind::OPERATOR;
  } else if (str.back() == ':') {
    kind = SymbolKind::KEYWORD;
  } else {
    kind = SymbolKind::IDENTIFIER;
  }
  return kind;
}

inline SymbolKind symbol_kind(const ast::Symbol& symbol) {
  return symbol_kind(symbol.str());
}

class Comment : public NodeInstance<Kind::COMMENT> {
 public:
  Comment(std::string value) : _location(), _value(value) {}

  Comment(LocationRange location, std::string value)
      : _location(location), _value(value) {}

  Comment(Location start, Location end, std::string value)
      : _location{start, end}, _value(value) {}

  const std::string& str() const { return _value; }

  const char* c_str() const { return str().c_str(); }

  const LocationRange& location() const { return _location; }

 private:
  LocationRange _location;
  std::string _value;
};

class String : public NodeInstance<Kind::STRING> {
 public:
  String(std::string value) : _location(), _value(value) {}

  String(LocationRange location, std::string value)
      : _location(location), _value(value) {}

  String(Location start, Location end, std::string value)
      : _location{start, end}, _value(value) {}

  const std::string& str() const { return _value; }

  const char* c_str() const { return str().c_str(); }

  const LocationRange& location() const { return _location; }

 private:
  LocationRange _location;
  std::string _value;
};

class Integer : public NodeInstance<Kind::INTEGER> {
 public:
  Integer(intptr_t value) : _location(), _value(value) {}

  Integer(LocationRange location, intptr_t value)
      : _location(location), _value(value) {}

  Integer(Location start, Location end, intptr_t value)
      : _location{start, end}, _value(value) {}

  intptr_t value() const { return _value; }

  const LocationRange& location() const { return _location; }

 private:
  LocationRange _location;
  std::intptr_t _value;
};

enum class SignatureKind { UNARY, BINARY, KEYWORD };

class Signature : public NodeInstance<Kind::SIGNATURE> {
 public:
  SignatureKind sigkind() { return _sigkind; }

 protected:
  Signature(SignatureKind k) : _sigkind(k) {}

 private:
  SignatureKind _sigkind;
};

template <SignatureKind K>
class SignatureInstance : public Signature {
 public:
  static constexpr SignatureKind SIGKIND = K;

 protected:
  SignatureInstance() : Signature(K) {}
};

using SignaturePtr = std::shared_ptr<Signature>;

template <typename T>
T* signature_cast(Signature* node) {
  static_assert(std::is_base_of_v<SignatureInstance<T::SIGKIND>, T>);
  assert(node->sigkind() == T::SIGKIND);
  return static_cast<T*>(node);
}

template <typename T>
T* signature_cast(Node* node) {
  assert(node->kind() == Kind::SIGNATURE);
  return signature_cast<T>(static_cast<Signature*>(node));
}

class UnarySignature : public SignatureInstance<SignatureKind::UNARY> {
 public:
  UnarySignature() = default;

  UnarySignature(const Symbol& symbol) : _symbol(symbol) {}

  UnarySignature(Symbol&& symbol) : _symbol(std::move(symbol)) {}

  void symbol(const Symbol& symbol) { _symbol = symbol; }

  void symbol(Symbol&& symbol) { _symbol = std::move(symbol); }

  const Symbol& symbol() const { return _symbol; }

 private:
  Symbol _symbol;
};

class BinarySignature : public SignatureInstance<SignatureKind::BINARY> {
 public:
  BinarySignature() = default;

  BinarySignature(const Symbol& symbol) : _symbol(symbol) {}

  BinarySignature(Symbol&& symbol) : _symbol(std::move(symbol)) {}

  void symbol(const Symbol& symbol) { _symbol = symbol; }

  void symbol(Symbol&& symbol) { _symbol = std::move(symbol); }

  const Symbol& symbol() const { return _symbol; }

  void argument(const Symbol& argument) { _argument = argument; }

  void argument(Symbol&& argument) { _argument = std::move(argument); }

  const Symbol& argument() const { return _argument; }

 private:
  Symbol _symbol;
  Symbol _argument;
};

struct KeywordArgument {
  Symbol keyword;
  Symbol argument;
};

class KeywordSignature : public SignatureInstance<SignatureKind::KEYWORD> {
 public:
  KeywordSignature() = default;

  std::vector<KeywordArgument>& arguments() { return _arguments; }

  const std::vector<KeywordArgument>& arguments() const { return _arguments; }

 public:
  std::vector<KeywordArgument> _arguments;
};

enum class ExpressionKind { UNARY, BINARY, SYMBOL, UNIT };

class Expression : public NodeInstance<Kind::EXPRESSION> {
 public:
  ExpressionKind exprkind() { return _exprkind; }

 protected:
  Expression(ExpressionKind k) : _exprkind(k) {}

 private:
  ExpressionKind _exprkind;
};

using ExpressionPtr = std::shared_ptr<Expression>;

template <ExpressionKind K>
class ExpressionInstance : public Expression {
 public:
  static constexpr ExpressionKind EXPRKIND = K;

 protected:
  ExpressionInstance() : Expression(K) {}
};

template <typename T>
T* expression_cast(Expression* node) {
  static_assert(std::is_base_of_v<ExpressionInstance<T::EXPRKIND>, T>);
  assert(node->exprkind() == T::EXPRKIND);
  return static_cast<T*>(node);
}

template <typename T>
T* expression_cast(Node* node) {
  assert(node->kind() == Kind::EXPRESSION);
  return signature_cast<T>(static_cast<Signature*>(node));
}

class UnaryExpression : public ExpressionInstance<ExpressionKind::UNARY> {
 public:
  UnaryExpression() = default;

  void receiver(const ExpressionPtr& receiver) { _receiver = receiver; }

  void receiver(ExpressionPtr&& receiver) { _receiver = std::move(receiver); }

  const ExpressionPtr& receiver() { return _receiver; }

  void message(const Symbol& message) { _message = message; }

  void message(Symbol&& message) { _message = std::move(message); }

  const Symbol& message() const { return _message; }

 private:
  ExpressionPtr _receiver;
  Symbol _message;
};

class BinaryExpression : public ExpressionInstance<ExpressionKind::BINARY> {
 public:
  BinaryExpression() = default;

  void receiver(const ExpressionPtr& receiver) { _receiver = receiver; }

  void receiver(ExpressionPtr&& receiver) { _receiver = std::move(receiver); }

  const ExpressionPtr& receiver() { return _receiver; }

  void message(const Symbol& message) { _message = message; }

  void message(Symbol&& message) { _message = std::move(message); }

  const Symbol& message() const { return _message; }

  void argument(const ExpressionPtr& argument) { _argument = argument; }

  void argument(ExpressionPtr&& argument) { _argument = std::move(argument); }

  const ExpressionPtr& argument() const { return _argument; }

 private:
  ExpressionPtr _receiver;
  Symbol _message;
  ExpressionPtr _argument;
};

class SymbolExpression : public ExpressionInstance<ExpressionKind::SYMBOL> {
 public:
  SymbolExpression() = default;

  void symbol(const Symbol& symbol) { _symbol = symbol; }

  void symbol(Symbol&& symbol) { _symbol = std::move(symbol); }

  const Symbol& symbol() const { return _symbol; }

 private:
  Symbol _symbol;
};

class UnitExpression : public ExpressionInstance<ExpressionKind::UNIT> {};

class Body : public NodeInstance<Kind::BODY> {
 public:
  void is_primitive(bool b) { _is_primitive = b; }

  bool is_primitive() { return _is_primitive; }

 private:
  bool _is_primitive = false;
};

class Method : public NodeInstance<Kind::METHOD> {
 public:
  virtual ~Method() override = default;

  const SignaturePtr& signature() const { return _signature; }

  void signature(const SignaturePtr& signature) { _signature = signature; }

  void signature(SignaturePtr&& signature) {
    _signature = std::move(signature);
  }

  const Body& body() const { return _body; }

  void body(const Body& body) { _body = body; }

  void body(Body&& body) { _body = std::move(body); }

 private:
  SignaturePtr _signature;
  Body _body;
};

using MethodPtr = std::shared_ptr<Method>;

using MethodList = std::vector<MethodPtr>;

// A smalltalk class
struct Clazz : public NodeInstance<Kind::CLAZZ> {
 public:
  Clazz() {}

  Clazz(Location start, Location end, Symbol name)
      : _location{start, end}, _name(name) {}

  Clazz(Location start, Location end, Symbol name, Symbol super)
      : _location{start, end}, _name(name), _super(super) {}

  const LocationRange& location() const { return _location; }

  Symbol& name() { return _name; }

  const Symbol& name() const { return _name; }

  void name(Symbol name) { _name = name; }

  MethodList& methods() { return _methods; }

  const MethodList& methods() const { return _methods; }

  SymbolList& locals() { return _locals; }

  const SymbolList& locals() const { return _locals; }

  std::optional<Symbol>& supCer() { return _super; }

  const std::optional<Symbol>& super() const { return _super; }

  void super(Symbol symbol) { _super = symbol; }

  void location(const LocationRange& location) { _location = location; }

  void location(LocationRange&& location) { _location = std::move(location); }

  const LocationRange& location() { return _location; }

 private:
  LocationRange _location;
  Symbol _name;
  std::optional<Symbol> _super;
  MethodList _methods;
  SymbolList _locals;
};

}  // namespace ast
}  // namespace omtalk

#endif