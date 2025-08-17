// Wait for the webpage to be fully loaded before running the script
document.addEventListener('DOMContentLoaded', () => {

    const startStopSelect = document.getElementById('start-stop');
    const endStopSelect = document.getElementById('end-stop');
    const timeInput = document.getElementById('start-time');
    const findRouteBtn = document.getElementById('find-route-btn');
    const resultsContainer = document.getElementById('results-container');

    // --- Function to fetch all stops from the C++ backend and populate the dropdowns ---
    async function populateStops() {
        try {
            // Call our C++ server's /api/stops endpoint
            const response = await fetch('http://localhost:8080/api/stops');
            const stops = await response.json();

            // Clear any placeholder options
            startStopSelect.innerHTML = '';
            endStopSelect.innerHTML = '';

            // Add each stop to both dropdown menus
            stops.forEach(stop => {
                const option1 = new Option(stop.name, stop.id);
                const option2 = new Option(stop.name, stop.id);
                startStopSelect.add(option1);
                endStopSelect.add(option2);
            });
        } catch (error) {
            resultsContainer.innerHTML = '<p style="color: red;">Error: Could not connect to the C++ backend server.</p>';
            console.error('Failed to fetch stops:', error);
        }
    }

    // --- Function to display the results on the page ---
    function displayResults(data) {
        resultsContainer.innerHTML = ''; // Clear previous results

        if (!data.results || data.results.length === 0) {
            resultsContainer.innerHTML = '<p>No path found for the selected route.</p>';
            return;
        }

        const header = document.createElement('h3');
        header.textContent = `Best Options from ${data.from} to ${data.to}:`;
        resultsContainer.appendChild(header);

        data.results.forEach(result => {
            const resultDiv = document.createElement('div');
            resultDiv.className = 'result-item';
            resultDiv.textContent = `Arrive at ${result.arrival_time} (${result.trips} trips)`;
            resultsContainer.appendChild(resultDiv);
        });
    }

    // --- Add a click event listener to the "Find Route" button ---
    findRouteBtn.addEventListener('click', async () => {
        const from = startStopSelect.value;
        const to = endStopSelect.value;
        const time = timeInput.value;

        if (!from || !to || !time) {
            alert('Please select a start, end, and time.');
            return;
        }

        resultsContainer.innerHTML = '<p>Searching...</p>';

        try {
            // Construct the URL and call our C++ backend's /api/route endpoint
            const response = await fetch(`http://localhost:8080/api/route?from=${from}&to=${to}&time=${time}`);
            const routeData = await response.json();
            displayResults(routeData);
        } catch (error) {
            resultsContainer.innerHTML = '<p style="color: red;">Error: Could not get route from the C++ backend server.</p>';
            console.error('Failed to fetch route:', error);
        }
    });

    // --- Initial setup ---
    populateStops(); // Populate the dropdowns as soon as the page loads
});
