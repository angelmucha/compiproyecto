# Proyecto Compiladores
## Integrantes
- Carlos Flores
- Fernando Choque
- Angel Mucha Huaman

# Punto 1: Comentarios

```cpp
  else if (c == '/')
  {
    c = nextChar();
    if (c == '/')
    {
      while (c != '\n' && c != '\0')
        c = nextChar();
      rollBack();
      return nextToken();
    }
    else
    {
      rollBack();
      token = new Token(Token::DIV);
    }
  } 
  ```

Para lograr la inclusión de comentarios en el código, se realizaron los siguientes cambios en el código del analizador léxico (scanner) y el analizador sintáctico (parser):

En el scanner:

1. Se agregó un nuevo caso en la función `nextToken()` para manejar el operador de división `/`.
2. Dentro del nuevo caso, se agregó un bloque de código para verificar si el siguiente carácter es también un `/`. Si es así, se omite todo el texto hasta el final de la línea actual.
3. Si no se encuentra un comentario de una sola línea, se crea un nuevo token de tipo `DIV` para el operador de división.
4. Se eliminó el caso existente para el operador de división `/` en el siguiente condicional else if (strchr("()+-*/;=<,!:", c)), ya que ahora se maneja en el nuevo caso.

Con estos cambios, el scanner ahora puede manejar comentarios de una sola línea en el código fuente y devolver el siguiente token válido después del comentario. 


# Punto 2: Generación de código

Se agregaron las siguientes definiciones de codegen en las implementaciones:

- `int ImpCodeGen::visit(ForStatement* s)`: Esta definición de codegen se utiliza para compilar la instrucción `for` en Imp. Genera código que inicializa la variable de iteración, compara el valor de la variable de iteración con el valor final y ejecuta el cuerpo del bucle. También genera código para incrementar la variable de iteración y volver a la comparación. Las etiquetas `l1` y `l2` se utilizan para controlar el flujo del bucle.
```cpp
int ImpCodeGen::visit(ForStatement* s) {
  string l1 = next_label();
  string l2 = next_label();
  string l3 = next_label();

  this->inicio=l1;  // Guarda la etiqueta de salida para break
  this->final=l2;  // Guarda la etiqueta de salida para break

  s->e1->accept(this);
  codegen(nolabel, "dup");
  s->e2->accept(this);
  codegen(nolabel, "lt");
  codegen(nolabel, "jmpz", l2);
  codegen(l1, "skip");
  s->body->accept(this);
  codegen(l3, "skip");
  codegen(nolabel, "push", 1);
  codegen(nolabel, "add");
  codegen(nolabel, "dup");
  s->e2->accept(this);
  codegen(nolabel, "lt");
  codegen(nolabel, "jmpz", l2);
  codegen(nolabel, "goto", l1);
  codegen(l2, "skip");

  this->inicio=""; 
  this->final="";  
  return 0;
}
```
- `int ImpCodeGen::visit(BoolConstExp* e)`: Esta definición de codegen se utiliza para compilar las constantes booleanas en Imp. Genera código que empuja un valor booleano a la pila.
```cpp
int ImpCodeGen::visit(BoolConstExp* e) { // agregado
  if (e->b) {
    codegen(nolabel,"push",1);
  } else {
    codegen(nolabel,"push",0);
  }
  return 0;
}
```
- `int ImpCodeGen::visit(BinaryExp* e)`: Esta definición de codegen se utiliza para compilar las expresiones binarias en Imp. Genera código que evalúa la expresión binaria y aplica el operador correspondiente. Se agregaron los casos `AND` y `OR` para los operadores lógicos.
```cpp
int ImpCodeGen::visit(BinaryExp* e) {
  e->left->accept(this);
  e->right->accept(this);
  string op = "";
  switch(e->op) {
  case PLUS: op =  "add"; break;
  case MINUS: op = "sub"; break;
  case MULT:  op = "mul"; break;
  case DIV:  op = "div"; break;
  case LT:  op = "lt"; break;
  case LTEQ: op = "le"; break;
  case EQ:  op = "eq"; break;
  case EXP:  op = "exp"; break;
  case AND:  op = "and"; break; // agregado
  case OR:  op = "or"; break; // agregado
  default: cout << "binop " << Exp::binopToString(e->op) << " not implemented" << endl;
  }
  codegen(nolabel, op);
  return 0;
}
```

# Punto 3: Sentencia do-while
El cambio que se realizó en la gramática de Imp fue agregar la regla DoWhileStatement para permitir la construcción de la instrucción `do-while`. La regla se define como: `DoWhileStatement ::= "do" Body "while" "(" Exp ")" ";"`
```cpp
// DoWhileStatement ::= "do" Body "while" "(" Exp ")" ";"
class DoWhileStatement : public Stm {
public:
  Body *body;
  Exp* cond;
  DoWhileStatement(Body* b, Exp* c);
  int accept(ImpVisitor* v);
  void accept(TypeVisitor* v);
  ~DoWhileStatement();
};
```
En el código del parser, se agregó un nuevo bloque `else if` para analizar la instrucción `do-while`. El bloque se encuentra en la línea 158 y se define como:
```cpp
else if (match(Token::DO))
  {
    tb = parseBody();
    if (!match(Token::WHILE))
      parserError("Esperaba WHILE en do-while");
    if (!match(Token::LPAREN))
      parserError("Esperaba ( en do-while");
    e = parseExp();
    if (!match(Token::RPAREN))
      parserError("Esperaba ) en do-while");
    s = new DoWhileStatement(tb, e);
  }
```

En el código de ImpTypeChecker, se agregó un nuevo método `visit` para la instrucción `DoWhileStatement`. El método se encuentra en la línea 89 y se define como:
```cpp
void ImpTypeChecker::visit(DoWhileStatement* s) {
  in_loop = true;
  s->body->accept(this);
  if (!s->cond->accept(this).match(booltype)) {
    cout << "Condicional en DoWhileStm debe de ser: " << booltype << endl;
    exit(0);
  }  
 in_loop = false;
 return;
}
```
En el código de `ImpCodeGen`, se agregó un nuevo método `visit` para la instrucción `DoWhileStatement`. El método se encuentra en la línea 104 y se define como:
```cpp
int ImpCodeGen::visit(DoWhileStatement *s)
{
  string l1 = next_label();
  string l2 = next_label();
  string l3 = next_label();

  this->inicio = l1; 
  this->final = l2;  

  codegen(l1, "skip");
  s->body->accept(this);
  codegen(l3, "skip"); 
  s->cond->accept(this);
  codegen(nolabel, "jmpz", l2);
  codegen(nolabel, "goto", l1);
  codegen(l2, "skip");

  this->inicio = ""; 
  this->final = ""; 
  return 0;
}
```

# Punto 4: Sentencias break y continue
Para agregar las sentencias break y continue primero añadimos los tokens nuevos para estas instrucciones.
```cpp
const char *Token::token_names[38] = { ..., "BREAK", "CONTINUE"}
```
Añadimos las palabras reservadas tambien:
```cpp
Scanner::Scanner(string s) : input(s), first(0), current(0)
{...
  reserved["break"] = Token::BREAK;
  reserved["continue"] = Token::CONTINUE;
}
```
Luego se añadio en el parsestatement las nuevas condiciones:
```cpp
else if (match(Token::BREAK))
  {
    s = new BreakStatement();
    if (!match(Token::SEMICOLON))
      parserError("Esperaba ; en break");
  }
  else if (match(Token::CONTINUE))
  {
    s = new ContinueStatement();
    if (!match(Token::SEMICOLON))
      parserError("Esperaba ; en continue");
  }
```
Añadimos al imp.hh los nuevos statements y con ello sus respectivos visitors:
```cpp
class BreakStatement : public Stm {
public:
  string label;
  BreakStatement();
  int accept(ImpVisitor* v);
  void accept(TypeVisitor* v);
  ~BreakStatement();
};

class ContinueStatement : public Stm {
public:
  string label;
  ContinueStatement();
  int accept(ImpVisitor* v);
  void accept(TypeVisitor* v);
  ~ContinueStatement();
};
```
Modificamos el typecheker con la ayuda de la variable global `in_loop` para verificar si estamos en un loop o no:

```cpp
void ImpTypeChecker::visit(BreakStatement* s) {
  if (!in_loop) {
    cout << "La instruccion break debe estar dentro de un bucle" << endl;
    exit(0);
  }
  return;
}

void ImpTypeChecker::visit(ContinueStatement* s) {
  if (!in_loop) {
    cout << "La instruccion continue debe estar dentro de un bucle" << endl;
    exit(0);
  }
  return;
}

```
Esta variable se actualizará en los bucles, se puede ver como cambia la declaracion de `in_loop` antes y despues de cada bucle:
```cpp
void ImpTypeChecker::visit(WhileStatement* s) {
  in_loop = true;
  if (!s->cond->accept(this).match(booltype)) {
    cout << "Condicional en WhileStm debe de ser: " << booltype << endl;
    exit(0);
  }  
  s->body->accept(this);
  in_loop = false;
 return;
}

void ImpTypeChecker::visit(ForStatement* s) {
  in_loop = true;
  ImpType t1 = s->e1->accept(this);
  ImpType t2 = s->e2->accept(this);
  if (!t1.match(inttype) || !t2.match(inttype)) {
    cout << "Tipos de rangos en for deben de ser: " << inttype << endl;
    exit(0);
  }
  env.add_level();
  env.add_var(s->id,inttype);
  s->body->accept(this);
  env.remove_level();
  in_loop = false;
 return;
}

void ImpTypeChecker::visit(DoWhileStatement* s) {
  in_loop = true;
  s->body->accept(this);
  if (!s->cond->accept(this).match(booltype)) {
    cout << "Condicional en DoWhileStm debe de ser: " << booltype << endl;
    exit(0);
  }  
 in_loop = false;
 return;
}
```

Para el codegen agregamos dos variables globales en las que almacenaremos los labels para el salto respectivo del break y continue:
```cpp
class ImpCodeGen : public ImpVisitor {
...
  private:
  ...
  string inicio,final;
  ...
}
```
Luego implementamos el codegen del break y continue según la lógica que conocemos:
```cpp
int ImpCodeGen::visit(BreakStatement *s)
{
  codegen(nolabel, "goto", this->final); // El break saltará hacia el label que marca el fin del bucle

  return 0;
}
int ImpCodeGen::visit(ContinueStatement *s)
{
  codegen(nolabel, "goto", this->inicio); // El continue saltará hacia el label del principio para que continue con la siguiente iteracion
  return 0;
}
```
Hacemos las respectivas modificaciones en whiledo, dowhile y for para que respondan a los saltos del break y continue:
```cpp
int ImpCodeGen::visit(WhileStatement *s)
{
  string l1 = next_label();
  string l2 = next_label();

  this->inicio = l1; 
  this->final = l2;  

  codegen(l1, "skip");
  s->cond->accept(this);
  codegen(nolabel, "jmpz", l2);
  s->body->accept(this);
  codegen(nolabel, "goto", l1);
  codegen(l2, "skip");

  this->inicio = ""; 
  this->final = "";  

  return 0;
}

int ImpCodeGen::visit(ForStatement *s)
{
  string l1 = next_label();
  string l2 = next_label();
  string l3 = next_label();

  this->inicio = l1;
  this->final = l2; 

  s->e1->accept(this);
  codegen(nolabel, "dup");
  s->e2->accept(this);
  codegen(nolabel, "lt");
  codegen(nolabel, "jmpz", l2);
  codegen(l1, "skip");
  s->body->accept(this);
  codegen(l3, "skip");
  codegen(nolabel, "push", 1);
  codegen(nolabel, "add");
  codegen(nolabel, "dup");
  s->e2->accept(this);
  codegen(nolabel, "lt");
  codegen(nolabel, "jmpz", l2);
  codegen(nolabel, "goto", l1);
  codegen(l2, "skip");

  this->inicio = "";
  this->final = ""; 
  return 0;
}
int ImpCodeGen::visit(DoWhileStatement *s)
{
  string l1 = next_label();
  string l2 = next_label();
  string l3 = next_label();

  this->inicio = l1; 
  this->final = l2;  

  codegen(l1, "skip");
  s->body->accept(this);
  codegen(l3, "skip"); // Agregado para gestionar la salida del cuerpo antes de la condición
  s->cond->accept(this);
  codegen(nolabel, "jmpz", l2);
  codegen(nolabel, "goto", l1);
  codegen(l2, "skip");

  this->inicio = ""; 
  this->final = "";  
  return 0;
}
```
