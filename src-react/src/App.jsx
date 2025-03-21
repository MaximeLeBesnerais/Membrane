import { Button } from '@/components/ui/button';
import { Card, CardContent } from '@/components/ui/card';
import { cn } from '@/lib/utils';

const MembraneHomePage = () => {
  return (
    <div
      className={cn(
        'min-h-screen flex items-center justify-center',
        'bg-gradient-to-br from-black via-gray-800 to-gray-900', // Updated gradient
        'p-4 sm:p-6'
      )}
    >
      <Card
        className={cn(
          'w-full max-w-md',
          'bg-gray-900/90 backdrop-blur-md', // Slightly transparent, dark background
          'border border-gray-800', // Darker border
          'shadow-2xl',
          'rounded-3xl',
          'transition-all duration-300',
          'hover:scale-[1.01] hover:shadow-gray-700/20', // Darker hover shadow
          'p-4 sm:p-6'
        )}
      >
        <CardContent className="space-y-4">
          <h1
            className={cn(
              'text-2xl sm:text-3xl md:text-4xl font-bold',
              'text-center',
              'bg-gradient-to-r from-gray-300 to-gray-400', // Lighter text gradient
              'text-transparent bg-clip-text',
              'drop-shadow-lg'
            )}
          >
            React + Membrane
          </h1>
          <div className="text-center">
            <p
              className={cn(
                'text-sm sm:text-base',
                'text-gray-400', // Lighter text color
                'mb-6 sm:mb-8',
                'max-w-[90%] mx-auto',
                'leading-relaxed'
              )}
            >
              Building the technology of tomorrow
            </p>
            <Button
              variant="outline"
              size="lg"
              className={cn(
                'bg-gradient-to-r from-gray-800/20 to-gray-700/20', // Darker button gradient
                'text-gray-300', // Lighter text color
                'border-2 border-gray-800', // Darker border
                'hover:from-gray-700/30 hover:to-gray-600/30',
                'hover:border-gray-700/30',
                'shadow-lg hover:shadow-gray-700/30', // Darker hover shadow
                'transition-all duration-300',
                'px-6 sm:px-8',
                'py-2.5 sm:py-3.5',
                'font-semibold',
                'rounded-full'
              )}
            >
              Get Started
            </Button>
          </div>
        </CardContent>
      </Card>
    </div>
  );
};

export default MembraneHomePage;
