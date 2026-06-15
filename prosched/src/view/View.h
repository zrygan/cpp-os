#ifndef VIEW_H
#define VIEW_H

/**
 * @brief The View class handles all display logic for the CSOPESY
 *  commandline interface. It delegates page rendering to the
 *  appropriate page functions (e.g., Menu).
 */
class View {
public:
  /**
   * @brief Displays the main menu of the CSOPESY commandline.
   *  Delegates to the Welcome() function defined in Menu.
   */
  void DisplayMenu();

  /**
   * @brief Clears the terminal screen.
   */
  void DisplayClear();
};

#endif