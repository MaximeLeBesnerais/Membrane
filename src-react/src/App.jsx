import { useState, useEffect } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from './assets/vite.svg'
import membraneLogo from './assets/membrane.svg'
import './App.css'
import Membrane from './Membrane'

function App() {
    const [count, setCount] = useState(0)
    const [currentTime, setCurrentTime] = useState(null)

    useEffect(() => {

        // Return cleanup function
        return () => {
            console.log("App component unmounting");
            // Any cleanup needed when this component unmounts
        };
    }, []);

    return (
        <>
            <div>
                <a href="https://vitejs.dev" target="_blank" rel="noreferrer">
                    <img src={viteLogo} className="logo" alt="Vite logo" />
                </a>
                <a href="https://react.dev" target="_blank" rel="noreferrer">
                    <img src={reactLogo} className="logo react" alt="React logo" />
                </a>
                <a href="https://https://github.com/MaximeLeBesnerais/Membrane" target="_blank" rel="noreferrer">
                    <img src={membraneLogo} className="logo membrane" alt="Membrane logo" width={128} />
                </a>
            </div>
            <h1>Vite + React + Membrane</h1>
            <div className="card">
                <button onClick={() => setCount((count) => count + 1)}>
                    count is {count}
                </button>
                {currentTime && (
                    <p>Current time from C++: {currentTime}</p>
                )}
                <p>
                    Edit <code>src/App.jsx</code> and save to test HMR
                </p>
            </div>
            <p className="read-the-docs">
                Click on the Vite and React logos to learn more
            </p>
        </>
    )
}

export default App