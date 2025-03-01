import { useEffect } from 'react'
import { Tldraw } from 'tldraw'
import 'tldraw/tldraw.css'
import './App.css'

function App() {
    useEffect(() => {
        return () => {
            console.log("App component unmounting");
        };
    }, []);

    return (
        <div style={{ position: 'fixed', inset: 0}}>
            <Tldraw />
        </div>
    )
}

export default App