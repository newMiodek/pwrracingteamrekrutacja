#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <vector>

// Ten enum zaznacza, w jakim rodzaju miejsca znajduje się dany punkt
// OUTSIDE: punkt jest poza dotychczas ustaloną zagrodą
// INSIDE_QUARTER: punkt znalazł się po wewnętrznej stronie przekątnej
// jakiejś ćwiartki, ale jako że może należeć do więcej niż jednej
// ćwiartki, to możliwe, że jeszcze stanie się częścią zagrody
// INSIDE_SHAPE: punkt napewno nie jest częścią zagrody - leży wewnątrz niej
// BOUNDARY: punkt jest częścią zagrody
enum Location {
  OUTSIDE,
  INSIDE_QUARTER,
  INSIDE_SHAPE,
  BOUNDARY
};

// Ten enum zaznacza, w której ćwiartce dzieją się operacje.
// TOP oznacza, że w top_right albo top_left.
// BOTTOM oznacza, że w bottom_right albo bottom_left.
enum WhichHalf {
  TOP,
  BOTTOM
};

// Ten enum jest potrzebny do rozróżnienia, w którą stronę idzie linia
// w przypadku, gdy jest ona pionowa. Kierunek linii pomaga wtedy poprawnie
// obliczyć wynik funkcji is_point_above_line oraz is_point_below_line,
// przy czym należy wtedy przymknąć oko na słowa "above" i "below" - po prostu
// punkt ma być po konkretnej stronie.
enum LineDirection {
  UP,
  DOWN
};

class Point {
private:
  double x;
  double y;
  Location loc;
public:
  double get_x() const { return this->x; }
  double get_y() const { return this->y; }
  Location get_loc() const { return this->loc; }

  void set_loc(Location loc) {
    this->loc = loc;
  }

  // Wszystkie punkty domyślnie są traktowane jakby były poza zagrodą
  Point(double x, double y) : x(x), y(y), loc(OUTSIDE) {}

  std::string to_string() const {
    return "(" + std::to_string(this->x) + ", " + std::to_string(this->y) + ")";
  }
};

class Line {
private:
  // Gdy valid == true to pole xint jest niezainicjowane
  // Gdy valid == false to pola grad i yint są niezainicjowane
  // Należy na to uważać
  double grad;
  double yint;
  bool valid;
  LineDirection dir;
  double xint; // Używane zamiast "grad" i "yint" gdy linia jest pionowa
public:
  Line(const Point& p1, const Point& p2) {
    this->valid = p1.get_x() != p2.get_x();
    if(valid) {
      this->grad = (p2.get_y() - p1.get_y()) / (p2.get_x() - p1.get_x());
      this->yint = p1.get_y() - this->grad * p1.get_x();
      if(grad >= 0) {
        this->dir = UP;
      }
      else {
        this->dir = DOWN;
      }
    }
    else {
      if(p1.get_y() > p2.get_y()) {
        this->dir = DOWN;
      }
      else {
        this->dir = UP;
      }
      this->xint = p1.get_x();
    }
  } 

  // Jeśli valid == false to zamiast "above" jest równoważność "po lewej stronie
  // patrząc wraz z kierunkiem linii"
  bool is_point_above_line(const Point& p) const {
    if(valid) {
      return p.get_y() > this->grad * p.get_x() + this->yint;
    }
    // else if !valid
    if(this->dir == UP) {
      return p.get_x() < this->xint;
    }
    // else if dir == DOWN
    return p.get_x() > this->xint;
  }
  // Jeśli valid == false to zamiast "below" jest równoważność "po prawej stronie
  // patrząc wraz z kierunkiem linii"
  bool is_point_below_line(const Point& p) const {
    if(valid) {
      return p.get_y() < this->grad * p.get_x() + this->yint;
    }
    // else if !valid
    if(this->dir == UP) {
      return p.get_x() > this->xint;
    }
    // else if dir == DOWN
    return p.get_x() < this->xint;
  }
};

class Box {
private:
  double top;
  double bottom;
  double left;
  double right;
  Line diagonal;
public:
  Line get_diagonal() const { return this->diagonal; }

  Box(const Point& p1, const Point& p2) : diagonal(p1, p2) {
    if(p1.get_x() > p2.get_x()) {
      this->right = p1.get_x();
      this->left = p2.get_x();
    }
    else {
      this->right = p2.get_x();
      this->left = p1.get_x();
    }
    if(p1.get_y() > p2.get_y()) {
      this->top = p1.get_y();
      this->bottom = p2.get_y();
    }
    else {
      this->top = p2.get_y();
      this->bottom = p1.get_y();
    }
  }

  bool does_contain(const Point& p) const {
    return p.get_x() >= this->left &&
           p.get_x() <= this->right &&
           p.get_y() >= this->bottom &&
           p.get_y() <= this->top;
  }
};

// Te funkcje mają być wprowadzane jako argumenty do funkcji std::sort

// Wprowadzenie poniższej funkcji poskutkuje posortowaniem punktów
// od największej wartości y do najmniejszej
bool compare_top_down(Point* p1, Point* p2) {
  return p1->get_y() > p2->get_y();
}
// Wprowadzenie poniższej funkcji poskutkuje posortowaniem punktów
// od najmniejszej wartości y do największej
bool compare_bottom_up(Point* p1, Point* p2) {
  return p1->get_y() < p2->get_y();
}

// Stwierdza czy kąt między podanymi punktami jest wklęsły
bool is_angle_concave(const Point& second_previous, const Point& previous, const Point& current, WhichHalf wh) {
  Line line(second_previous, current);
  if(wh == TOP) {
    return line.is_point_below_line(previous);
  }
  // else if(wh == BOTTOM)
  return line.is_point_above_line(previous);
}

// Ta funkcja cofa się po punktach w "quarter_boundary_points"
// i usuwa napotkane punkty tworzące wklęsłości.
// Zatrzymuje się na pierwszej wypukłości lub gdy w "quarter_boundary_points"
// są mniej niż 4 punkty.
void remove_concavities(std::list<Point*>& quarter_boundary_points, const std::list<Point*>::iterator& current, WhichHalf wh) {
  if(quarter_boundary_points.size() < 4) {
    // size == 2 oznaczałby, że w liście są tylko punkty skrajne danej ćwiartki
    // size == 3 oznaczałby, że w liście są tylko punkty skrajne i jeden punkt,
    // a jeśli zaszedł on do tego momentu to napewno nie tworzy wklęsłości
    // między punktami skrajnymi
    // dopiero dla size >= 4 mogą się zdarzyć wklęsłości
    return;
  }
  std::list<Point*>::iterator previous = current;
  previous--;
  std::list<Point*>::iterator second_previous = previous;
  second_previous--;

  while(is_angle_concave(**second_previous, **previous, **current, wh)) {
    (**previous).set_loc(INSIDE_SHAPE);
    quarter_boundary_points.erase(previous);
    if(quarter_boundary_points.size() < 4) {
      return;
    }
    previous = second_previous;
    second_previous--;
  }
}

std::list<Point*> extract_boundary(std::vector<Point>& points) {
  if(points.size() <= 3) {
    std::list<Point*> boundary_points;
    for(int i = 0; i < points.size(); i++) {
      boundary_points.push_back(&points[i]);
    }
    return boundary_points;
  }

  // Wyznaczanie punktów skrajnych - one napewno są częścią zagrody
  int i_top = 0;
  int i_right = 0;
  int i_bottom = 0;
  int i_left = 0;
  for(int i = 1; i < points.size(); i++) {
    if(points[i].get_y() > points[i_top].get_y()) {
      i_top = i;
    }
    else if(points[i].get_y() < points[i_bottom].get_y()) {
      i_bottom = i;
    }
    if(points[i].get_x() > points[i_right].get_x()) {
      i_right = i;
    }
    else if(points[i].get_x() < points[i_left].get_x()) {
      i_left = i;
    }
  }
  points[i_top].set_loc(BOUNDARY);
  points[i_right].set_loc(BOUNDARY);
  points[i_bottom].set_loc(BOUNDARY);
  points[i_left].set_loc(BOUNDARY);

  Box top_right_box(points[i_top], points[i_right]);
  Box bottom_right_box(points[i_bottom], points[i_right]);
  Box top_left_box(points[i_left], points[i_top]);
  Box bottom_left_box(points[i_left], points[i_bottom]);
  std::vector<Point*> top_right_points;
  std::vector<Point*> bottom_right_points;
  std::vector<Point*> top_left_points;
  std::vector<Point*> bottom_left_points;
  top_right_points.reserve(points.size());
  bottom_right_points.reserve(points.size());
  top_left_points.reserve(points.size());
  bottom_left_points.reserve(points.size());

  // Sprawdzanie, w których ćwiartkach leżą punkty
  for(int i = 0; i < points.size(); i++) {
    if(points[i].get_loc() == BOUNDARY) {
      continue;
    }
    if(top_right_box.does_contain(points[i])) {
      if(top_right_box.get_diagonal().is_point_below_line(points[i])) {
        points[i].set_loc(INSIDE_QUARTER);
      }
      else {
        top_right_points.push_back(&points[i]);
      }
    }
    if(bottom_right_box.does_contain(points[i])) {
      if(bottom_right_box.get_diagonal().is_point_above_line(points[i])) {
        points[i].set_loc(INSIDE_QUARTER);
      }
      else {
        bottom_right_points.push_back(&points[i]);
      }
    }
    if(top_left_box.does_contain(points[i])) {
      if(top_left_box.get_diagonal().is_point_below_line(points[i])) {
        points[i].set_loc(INSIDE_QUARTER);
      }
      else {
        top_left_points.push_back(&points[i]);
      }
    }
    if(bottom_left_box.does_contain(points[i])) {
      if(bottom_left_box.get_diagonal().is_point_above_line(points[i])) {
        points[i].set_loc(INSIDE_QUARTER);
      }
      else {
        bottom_left_points.push_back(&points[i]);
      }
    }
  }

  // Sprawdzenie dla prawej górnej ćwiartki;
  // pozostałe działają analogicznie
  if(!top_right_points.empty()) {
    if(top_right_points.size() == 1) {
      // Jeśli jest tu tylko jeden punkt to napewno nie tworzy wklęsłości
      // między punktami skrajnymi tej ćwiartki - gdyby tworzył to zostałby
      // odtrącony w powyższej pętli.
      top_right_points[0]->set_loc(BOUNDARY);
    }
    else {
      std::sort(top_right_points.begin(), top_right_points.end(), compare_top_down);
      // W tą listę są zbierane punkty będące częścią zagrody
      // spośród punktów w tej ćwiartce
      std::list<Point*> quarter_boundary_points {&points[i_top], &points[i_right]};
      std::list<Point*>::iterator last = quarter_boundary_points.end();
      last--;
      double rightmost_x = points[i_top].get_x();
      for(int i = 0; i < top_right_points.size(); i++) {
        if(top_right_points[i]->get_x() < rightmost_x) {
          // Jeśli punkt jest na lewo od ostatnio dodanego do zagrody punktu
          // to znajduje się wewnątrz zagrody (oprócz specjalnych przypadków).
          if(top_right_points[i]->get_y() == points[i_top].get_y()) {
            // Specjalny przypadek: punkty leżące na poziomie skrajnie górnego
            // punktu są współliniowe i wszystkie są częścią zagrody.
            top_right_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, top_right_points[i]);
            // Nie trzeba wywoływać funkcji remove_concavities,
            // bo idąc od góry takie punkty współliniowe ze skrajnie górnym
            // nie tworzą wklęsłości
          }
          else {
            top_right_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else if(top_right_points[i]->get_x() == rightmost_x) {
          // Jeśli punkt jest na tym samym X co poprzednio dodany
          // do zagrody punkt to znajduje się wewnątrz zagrody
          // (oprócz specjalnych przypadków).
          if(top_right_points[i]->get_x() == points[i_right].get_x()) {
            // Specjalny przypadek: punkty leżące na tym samym X co skrajnie
            // prawy punkt są współliniowe i wszystkie są częścią zagrody.
            top_right_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, top_right_points[i]);

            std::list<Point*>::iterator current = last;
            current--;
            remove_concavities(quarter_boundary_points, current, TOP);
          }
          else {
            top_right_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else {
          // Punkt na prawo od ostatnio dodatego punktu może być częścią zagrody
          top_right_points[i]->set_loc(BOUNDARY);
          quarter_boundary_points.insert(last, top_right_points[i]);
          rightmost_x = top_right_points[i]->get_x();
          
          std::list<Point*>::iterator current = last;
          current--;
          remove_concavities(quarter_boundary_points, current, TOP);
        }
      }
      remove_concavities(quarter_boundary_points, last, TOP);
    }
  }

  if(!bottom_right_points.empty()) {
    if(bottom_right_points.size() == 1) {
      bottom_right_points[0]->set_loc(BOUNDARY);
    }
    else {
      std::sort(bottom_right_points.begin(), bottom_right_points.end(), compare_bottom_up);
      std::list<Point*> quarter_boundary_points {&points[i_bottom], &points[i_right]};
      std::list<Point*>::iterator last = quarter_boundary_points.end();
      last--;
      double rightmost_x = points[i_bottom].get_x();
      for(int i = 0; i < bottom_right_points.size(); i++) {
        if(bottom_right_points[i]->get_x() < rightmost_x) {
          if(bottom_right_points[i]->get_y() == points[i_bottom].get_y()) {
            bottom_right_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, bottom_right_points[i]);
          }
          else {
            bottom_right_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else if(bottom_right_points[i]->get_x() == rightmost_x) {
          if(bottom_right_points[i]->get_x() == points[i_right].get_x()) {
            bottom_right_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, bottom_right_points[i]);

            std::list<Point*>::iterator current = last;
            current--;
            remove_concavities(quarter_boundary_points, current, BOTTOM);
          }
          else {
            bottom_right_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else {
          bottom_right_points[i]->set_loc(BOUNDARY);
          quarter_boundary_points.insert(last, bottom_right_points[i]);
          rightmost_x = bottom_right_points[i]->get_x();
          
          std::list<Point*>::iterator current = last;
          current--;
          remove_concavities(quarter_boundary_points, current, BOTTOM);
        }
      }
      remove_concavities(quarter_boundary_points, last, BOTTOM);
    }
  }

  if(!top_left_points.empty()) {
    if(top_left_points.size() == 1) {
      top_left_points[0]->set_loc(BOUNDARY);
    }
    else {
      std::sort(top_left_points.begin(), top_left_points.end(), compare_top_down);
      std::list<Point*> quarter_boundary_points {&points[i_top], &points[i_left]};
      std::list<Point*>::iterator last = quarter_boundary_points.end();
      last--;
      double leftmost_x = points[i_top].get_x();
      for(int i = 0; i < top_left_points.size(); i++) {
        if(top_left_points[i]->get_x() > leftmost_x) {
          if(top_left_points[i]->get_y() == points[i_top].get_y()) {
            top_left_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, top_left_points[i]);
          }
          else {
            top_left_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else if(top_left_points[i]->get_x() == leftmost_x) {
          if(top_left_points[i]->get_x() == points[i_left].get_x()) {
            top_left_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, top_left_points[i]);

            std::list<Point*>::iterator current = last;
            current--;
            remove_concavities(quarter_boundary_points, current, TOP);
          }
          else {
            top_left_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else {
          top_left_points[i]->set_loc(BOUNDARY);
          quarter_boundary_points.insert(last, top_left_points[i]);
          leftmost_x = top_left_points[i]->get_x();
          
          std::list<Point*>::iterator current = last;
          current--;
          remove_concavities(quarter_boundary_points, current, TOP);
        }
      }
      remove_concavities(quarter_boundary_points, last, TOP);
    }
  }

  if(!bottom_left_points.empty()) {
    if(bottom_left_points.size() == 1) {
      bottom_left_points[0]->set_loc(BOUNDARY);
    }
    else {
      std::sort(bottom_left_points.begin(), bottom_left_points.end(), compare_bottom_up);
      std::list<Point*> quarter_boundary_points {&points[i_bottom], &points[i_left]};
      std::list<Point*>::iterator last = quarter_boundary_points.end();
      last--;
      double leftmost_x = points[i_bottom].get_x();
      for(int i = 0; i < bottom_left_points.size(); i++) {
        if(bottom_left_points[i]->get_x() > leftmost_x) {
          if(bottom_left_points[i]->get_y() == points[i_bottom].get_y()) {
            bottom_left_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, bottom_left_points[i]);
          }
          else {
            bottom_left_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else if(bottom_left_points[i]->get_x() == leftmost_x) {
          if(bottom_left_points[i]->get_x() == points[i_left].get_x()) {
            bottom_left_points[i]->set_loc(BOUNDARY);
            quarter_boundary_points.insert(last, bottom_left_points[i]);

            std::list<Point*>::iterator current = last;
            current--;
            remove_concavities(quarter_boundary_points, current, BOTTOM);
          }
          else {
            bottom_left_points[i]->set_loc(INSIDE_SHAPE);
          }
        }
        else {
          bottom_left_points[i]->set_loc(BOUNDARY);
          quarter_boundary_points.insert(last, bottom_left_points[i]);
          leftmost_x = bottom_left_points[i]->get_x();
          
          std::list<Point*>::iterator current = last;
          current--;
          remove_concavities(quarter_boundary_points, current, BOTTOM);
        }
      }
      remove_concavities(quarter_boundary_points, last, BOTTOM);
    }
  }

  std::list<Point*> boundary_points;
  for(int i = 0; i < points.size(); i++) {
    if(points[i].get_loc() == BOUNDARY) {
      boundary_points.push_back(&points[i]);
    }
  }

  return boundary_points;
}

void print_boundary_points(const std::list<Point*>& boundary_points) {
  std::cout << "[";
  int i = 0;
  std::list<Point*>::const_iterator it = boundary_points.begin();
  while(i < boundary_points.size()) {
    std::cout << (**it).to_string();
    if(i != boundary_points.size() - 1) {
      std::cout << ", ";
    }
    i++;
    it++;
  }
  for(std::list<Point*>::const_iterator it = boundary_points.begin(); it != boundary_points.end(); it++);
  std::cout << "]" << std::endl;
}

void read_points(char* filename, std::vector<Point>* points) {
  std::string line;
  std::ifstream fin;

  fin.open(filename);
  if(!fin.good()) {
    std::cout << "Błąd: Nie znaleziono pliku o nazwie \"" << filename << "\"" << std::endl;
    exit(1);
  }

  if(!std::getline(fin, line)) {
    std::cout << "Błąd: Plik zawiera niewłaściwe dane" << std::endl;
    exit(1);
  }
  int point_count = std::stoi(line);

  points->reserve(point_count);
  for(int i = 0; i < point_count; i++) {
    if(!std::getline(fin, line)) {
      std::cout << "Błąd: Plik zawiera niewłaściwe dane" << std::endl;
      exit(1);
    }
    std::size_t space_index;
    double x = std::stof(line, &space_index);
    double y = std::stof(line.substr(space_index, line.size() - space_index));
    points->push_back(Point(x, y));
  }

  fin.close();
}

int main(int argc, char** argv) {
  // Pierwszy argument to ma być nazwa pliku z danymi.
  if(argc != 2) {
    std::cout << "Błąd: Podaj dokładnie jeden argument - nazwę pliku do odczytu" << std::endl;
    exit(1);
  }

  std::vector<Point> points;
  read_points(argv[1], &points);

  std::list<Point*> boundary_points = extract_boundary(points);

  print_boundary_points(boundary_points);

  return 0;
}

