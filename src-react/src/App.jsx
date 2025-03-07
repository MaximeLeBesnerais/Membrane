import React from 'react';
import { Code, Cpu, ExternalLink } from 'lucide-react';

const ReactMembranePage = () => {
  return (
    <div className="flex flex-col items-center justify-center min-h-screen bg-gradient-to-br from-gray-900 to-black text-white p-6">
      <div className="max-w-3xl w-full bg-gray-800 rounded-xl shadow-2xl overflow-hidden">
        <div className="p-8">
          <div className="flex items-center justify-center space-x-4 mb-8">
            <div className="bg-blue-500 p-4 rounded-full">
              <Code size={36} />
            </div>
            <span className="text-3xl font-bold">+</span>
            <div className="bg-purple-600 p-4 rounded-full">
              <Cpu size={36} />
            </div>
          </div>
          
          <h1 className="text-4xl font-bold text-center mb-6 bg-gradient-to-r from-blue-400 to-purple-500 bg-clip-text text-transparent">
            React + Membrane
          </h1>
          
          <div className="text-center text-gray-300 mb-8">
            A powerful combination for building modern web applications
          </div>
          
          <div className="flex justify-center">
            <button className="flex items-center space-x-2 bg-gradient-to-r from-blue-500 to-purple-600 px-6 py-3 rounded-lg text-white font-semibold hover:opacity-90 transition-opacity">
              <span>Get Started</span>
              <ExternalLink size={18} />
            </button>
          </div>
        </div>
        
        <div className="bg-gray-900 p-6 text-center text-gray-400 text-sm">
          Building the future of web development
        </div>
      </div>
    </div>
  );
};

export default ReactMembranePage;
