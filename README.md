# QNX Timer with CURL - Fetch Quote Example

## Main points
- Creates a timer using `timer_create()` and pulses to handle periodic events.
- Uses libcurl to fetch a quote from the Kanye REST API.
- Extracts and prints the quote from a JSON response.

## Dependencies
- **QNX Neutrino**: The program is designed to work in a QNX environment.
- **libcurl**: For making HTTP requests to the Kanye API.
