# Shared Memory Access Control System

This program implements a solution to the reader-writer synchronization problem with authentication using POSIX threads and semaphores in C on Linux.

## Features

- Implements shared memory access control with authentication
- Supports both authenticated and unauthenticated participants
- Uses semaphores for synchronization
- Implements authentication system for participant verification
- Supports 1-9 readers and writers
- Each participant performs 5 operations
- 1-second delay between operations
- Writers generate random numbers (0-9999)

## Requirements

- Linux operating system
- GCC compiler
- POSIX threads library

## Compilation

To compile the program, simply run:

```bash
make
```

## Usage

Run the program with the number of readers and writers as arguments:

```bash
./reader_writer <number_of_readers> <number_of_writers>
```

Example:
```bash
./reader_writer 3 2
```

## Implementation Details

1. **Authentication System**:
   - Each authenticated participant registers in the authentication database
   - Thread IDs are converted to authentication tokens
   - Unauthenticated participants operate without registration

2. **Synchronization**:
   - Uses resource locks for mutual exclusion
   - Readers can access shared memory simultaneously
   - Writers have exclusive access to shared memory

3. **Output Format**:
   The program produces output in the following format:
   ```
   Participant_ID Auth_Token Status(authenticated/unauthenticated) Type(reader/writer) Value
   ```

## Cleanup

To clean up compiled files:

```bash
make clean
``` 