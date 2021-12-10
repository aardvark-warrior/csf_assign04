Arthur Wang - awang91
Mason Albert - malber20

We did both parts of the homework together for this assignment.

--------------
Synchronization
--------------

We used a mutex for synchronization in this project. We initialized and destroyed the mutex in the Calculator constructor and destructor,
respectively. We deemed the expression evaluation as a critical section, so we locked the mutex at the top of Calc::evalExpr, and unlocked right before the return statement.
This way the mutex was locked right before the expression was tokenized, and evaluated while updating variables and performing computations.
